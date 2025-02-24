/*!
\brief This demo demonstrates how Conway's Game of Life can be implemented efficiently using Compute in Vulkan.
\file VulkanGameOfLife.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

const char FragShaderSrcFile[] = "FragShader.fsh.spv";
const char VertShaderSrcFile[] = "VertShader.vsh.spv";
const char CompShaderSrcFile[] = "CompShader.csh.spv";

enum BoardConfig
{
	Random = 0,
	Checkerboard,
	SpaceShips,
	NumBoards
};

const char* boardConfigs[BoardConfig::NumBoards] = { "Random", "CheckerBoard", "SpaceShips" };

/// <summary>Resources used throughout the demo.</summary>
struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Queue queues[2];
	pvr::utils::vma::Allocator vmaAllocator;
	pvrvk::Swapchain swapchain;

	pvrvk::DescriptorPool descriptorPool;
	pvrvk::CommandPool cmdPool;
	pvrvk::CommandPool computeCmdPool;

	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	pvrvk::Semaphore computeToComputeSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore computeToRenderSemaphore[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore renderToComputeSemaphore[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	pvrvk::Fence computeFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

	// Two primary Command Buffers one for Compute and one for Graphics
	pvr::Multi<pvrvk::CommandBuffer> graphicsPrimaryCmdBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> uiRendererCmdBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> graphicsCmdBuffers;

	pvr::Multi<pvrvk::CommandBuffer> computePrimaryCmdBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> computeCmdBuffers;

	// Image views for Board and Petri Dish effect.
	pvr::Multi<pvrvk::ImageView> boardImageViews;
	pvrvk::ImageView petriDishImageView;

	// Compute descriptor sets
	pvr::Multi<pvrvk::DescriptorSet> computeDescriptorSets;

	// Graphics descriptor sets
	pvr::Multi<pvrvk::DescriptorSet> graphicsDescriptorSets;

	// Descriptor set layouts
	pvrvk::DescriptorSetLayout computeDescriptorSetLayout;
	pvrvk::DescriptorSetLayout graphicsDescriptorSetLayout;

	pvrvk::GraphicsPipeline graphicsPipeline;
	pvrvk::ComputePipeline computePipeline;

	pvrvk::PipelineLayout computePipelinelayout;
	pvrvk::PipelineLayout graphicsPipelinelayout;

	pvrvk::Sampler graphicsSampler;
	pvrvk::Sampler computeSampler;

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

/// <summary>VulkanGameOfLife is the main demo class implementing the PVRShell functions.</summary>
class VulkanGameOfLife : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;

	uint32_t currentFrameId;
	uint32_t previousFrameId;
	uint32_t renderComputeSyncId;

	uint32_t graphicsQueueIndex;
	uint32_t computeQueueIndex;

	bool useMultiQueue;
	unsigned int stepCount;

	std::vector<unsigned char> board;
	std::vector<unsigned char> petriDish;

	float zoomRatio;
	int zoomLevel;
	std::string zoomRatioUI;
	std::string boardConfigUI;
	int currBoardConfig;
	int generation;

	int boardWidth = 0;
	int boardHeight = 0;

	int boardOffSetX = 0;
	int boardOffSetY = 0;

public:
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

private:
	void setZoomLevel(int zoomLevel);
	virtual void eventMappedInput(pvr::SimplifiedInput key);
	void createPipelines();
	void createResources();
	void recordUICmdBuffer();
	void generateTextures(pvrvk::CommandBuffer& cmdBuffer);
	pvrvk::CommandBuffer recordGraphicsCmdBuffer(const uint32_t swapchainIndex);
	pvrvk::CommandBuffer recordComputeCmdBuffer();
	void submitComputeWork(pvrvk::CommandBuffer submitCmdBuffer);
	void submitGraphicsWork(pvrvk::CommandBuffer submitCmdBuffer);
	unsigned int getPetriDishSize() { return std::max(getHeight(), getWidth()) / 4; }
	void generateBoardData();
	void refreshBoard(bool regenData);
	void createPetriDishEffect(pvrvk::CommandBuffer& cmdBuffer);
	void updateDesciptorSets();

	// Pick a bit on the board and set it to either full(default) or empty.
	void setBoardBit(int x, int y, bool bit = true)
	{
		if ((x + boardOffSetX < boardWidth) && (x + boardOffSetX >= 0) && (y + boardOffSetY < boardHeight) && (y + boardOffSetY >= 0))
		{
			int idx = (y + boardOffSetY) * boardWidth + x + boardOffSetX;
			board[idx] = bit * 255;
		}
	}
	// Set an offset for the setBoardBit operation.
	void setBoardBitOffset(int x, int y)
	{
		boardOffSetX = x;
		boardOffSetY = y;
	}
};

/// <summary>Resets board texture data and restarts the simulation.</summary>
void VulkanGameOfLife::refreshBoard(bool regenData = false)
{
	_deviceResources->device->waitIdle();
	_deviceResources->graphicsPrimaryCmdBuffers[0]->reset();
	_deviceResources->graphicsPrimaryCmdBuffers[0]->begin();

	generateBoardData();

	if (regenData) { generateTextures(_deviceResources->graphicsPrimaryCmdBuffers[0]); }
	else
	{
		// Update board image with randomised data.
		pvr::utils::ImageUpdateInfo imgUpdateInfo;
		imgUpdateInfo.arrayIndex = 0;
		imgUpdateInfo.data = board.data();
		imgUpdateInfo.dataHeight = imgUpdateInfo.imageHeight = _deviceResources->boardImageViews[0]->getImage()->getHeight();
		imgUpdateInfo.dataWidth = imgUpdateInfo.imageWidth = _deviceResources->boardImageViews[0]->getImage()->getWidth();
		imgUpdateInfo.dataSize = static_cast<uint32_t>(board.size());

		for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
		{
			pvr::utils::updateImage(_deviceResources->device, _deviceResources->graphicsPrimaryCmdBuffers[0], &imgUpdateInfo, 1, pvrvk::Format::e_R8_UNORM,
				pvrvk::ImageLayout::e_GENERAL, false, _deviceResources->boardImageViews[i]->getImage());
		}
	}

	_deviceResources->graphicsPrimaryCmdBuffers[0]->end();

	pvrvk::SubmitInfo submit;
	submit.commandBuffers = &_deviceResources->graphicsPrimaryCmdBuffers[0];
	submit.numCommandBuffers = 1;
	_deviceResources->queues[0]->submit(&submit, 1);
	_deviceResources->queues[0]->waitIdle();

	_deviceResources->graphicsPrimaryCmdBuffers[0]->reset(pvrvk::CommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	if (regenData) { updateDesciptorSets(); }
}

/// <summary>Sets the ZoomLevel of the board by calculating the ZoomRatio.</summary>
/// <param name="zoomLvl"> desired level of zoom.</param>
void VulkanGameOfLife::setZoomLevel(int zoomLvl)
{
	// Updating the Zoom Ratio based on the current level.
	zoomLevel = zoomLvl;
	zoomRatio = zoomLevel > 0 ? zoomLevel : 1.0f / (-zoomLevel + 2.0f);

	boardWidth = static_cast<int>(getWidth() / zoomRatio);
	boardHeight = static_cast<int>(getHeight() / zoomRatio);
	board.resize(static_cast<size_t>(boardWidth * boardHeight));

	// Updating the Zoom UI Label and Value
	zoomRatioUI = "\nZoom Level : ";

	std::ostringstream oss;
	oss << std::setprecision(2) << zoomRatio;

	zoomRatioUI += oss.str();
}

/// <summary>Generates data as a starting state for the Game Of Life board.</summary>
void VulkanGameOfLife::generateBoardData()
{
	generation = 0;

	switch (currBoardConfig)
	{
	// Generates a board with random data.
	default:
	case BoardConfig::Random: {
		// Randomly Fill the board to create a starting state for simulation.
		for (unsigned int i = 0; i < board.size(); ++i)
		{
			if (pvr::randomrange(0, 1) > .75f) { board[i] = static_cast<unsigned char>(255); }
			else
			{
				board[i] = static_cast<unsigned char>(0);
			}
		}
	}
	break;

	// Generates a Checker board.
	case BoardConfig::Checkerboard: {
		int checkerSize = 5;
		// This function will generate a checkered texture on the fly to be used on the triangle that is going
		// to be rendered and rotated on screen.
		for (unsigned int i = 0; i < board.size(); ++i)
		{
			int row = i / boardWidth;

			bool rowblack = (row / checkerSize) % 2;
			bool colblack = ((i % boardWidth) / checkerSize) % 2;

			int r = rowblack ^ colblack ? 255 : 0;

			board[i] = static_cast<unsigned char>(r);
		}
	}
	break;

	// Generates a board with Heavyweight spaceship at random positions.
	case BoardConfig::SpaceShips: {
		memset(board.data(), 0, board.size());

		for (int i = 0; i < 200 / zoomRatio; ++i)
		{
			setBoardBitOffset((int)pvr::randomrange(0.f, (float)boardWidth), (int)pvr::randomrange(0.f, (float)boardHeight));

			if (rand() % 2 == 0)
			{ // HWSS
				setBoardBit(0, 0);
				setBoardBit(0, 1);
				setBoardBit(0, 2);
				setBoardBit(1, 0);
				setBoardBit(1, 3);
				setBoardBit(2, 0);
				setBoardBit(3, 0);
				setBoardBit(3, 4);
				setBoardBit(4, 0);
				setBoardBit(4, 4);
				setBoardBit(5, 0);
				setBoardBit(6, 1);
				setBoardBit(6, 3);
			}
			else
			{
				// HWSS
				setBoardBit(6, 0);
				setBoardBit(6, 1);
				setBoardBit(6, 2);
				setBoardBit(5, 0);
				setBoardBit(5, 3);
				setBoardBit(4, 0);
				setBoardBit(3, 0);
				setBoardBit(3, 4);
				setBoardBit(2, 0);
				setBoardBit(2, 4);
				setBoardBit(1, 0);
				setBoardBit(0, 1);
				setBoardBit(0, 3);
			}
		}

		// Acorn:
		// setBoardBit(0, 0);
		// setBoardBit(1, 0);
		// setBoardBit(1, 2);
		// setBoardBit(3, 1);
		// setBoardBit(4, 0);
		// setBoardBit(5, 0);
		// setBoardBit(6, 0);

		// ?
		// setBoardBit(0, 0, true);
		// setBoardBit(0, 1, true);
		// setBoardBit(0, 2, true);
		// setBoardBit(2, 0, true);
		// setBoardBit(2, 1, true);
		// setBoardBit(3, -1, true);
		// setBoardBit(3, 1, true);
		// setBoardBit(4, -1, true);
		// setBoardBit(4, 1, true);
		// setBoardBit(5, 0, true);

		// R-pentomino
		// setBoardBit(0, 1);
		// setBoardBit(1, 0);
		// setBoardBit(1, 1);
		// setBoardBit(1, 2);
		// setBoardBit(2, 2);

		// B-heptomino
		// setBoardBit(0, 1);
		// setBoardBit(0, 2);
		// setBoardBit(1, 0);
		// setBoardBit(1, 1);
		// setBoardBit(2, 1);
		// setBoardBit(2, 2);
		// setBoardBit(3, 2);

		// Pi-heptomino
		// setBoardBit(0, 0);
		// setBoardBit(0, 1);
		// setBoardBit(0, 2);
		// setBoardBit(1, 2);
		// setBoardBit(2, 0);
		// setBoardBit(2, 1);
		// setBoardBit(2, 2);
	}
	break;
	}
}

/// <summary>Creates the Petri dish effect texture.</summary>
void VulkanGameOfLife::createPetriDishEffect(pvrvk::CommandBuffer& cmdBuffer)
{
	unsigned int petriDishSize = getPetriDishSize();
	// Creating the Petri dish effect.
	petriDish.resize(static_cast<size_t>(petriDishSize * petriDishSize));

	float radius = petriDishSize * .5f;
	for (unsigned int y = 0; y < petriDishSize; y++)
	{
		for (unsigned int x = 0; x < petriDishSize; x++)
		{
			glm::vec2 r((float)(x - radius), (float)(y - radius));

			int i = (y * petriDishSize + x);
			petriDish[i] = (unsigned char)(std::max(0.f, std::min(255.f, (1.2f - glm::length(r) / radius) * 255.f)));
		}
	}

	// Create the Petri Dish Texture
	pvr::TextureHeader petritextureheader(pvr::PixelFormat::R_8(), petriDishSize, petriDishSize);
	pvr::Texture petriTexture(petritextureheader, petriDish.data());

	_deviceResources->petriDishImageView = pvr::utils::uploadImageAndView(_deviceResources->device, petriTexture, true, cmdBuffer,
		pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT, pvrvk::ImageLayout::e_GENERAL, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
}

/// <summary>Generates random textures as a starting state for the Game Of Life.</summary>
void VulkanGameOfLife::generateTextures(pvrvk::CommandBuffer& cmdBuffer)
{
	// Create the board textures.
	pvr::TextureHeader textureheader(pvr::PixelFormat::R_8(), boardWidth, boardHeight);
	pvr::Texture boardTexture(textureheader, board.data());

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); i++)
	{
		_deviceResources->boardImageViews[i] =
			pvr::utils::uploadImageAndView(_deviceResources->device, boardTexture, true, cmdBuffer, pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_STORAGE_BIT,
				pvrvk::ImageLayout::e_GENERAL, _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}
}

/// <summary> Updates descriptor sets with new Images for the compute and graphics stages.</summary>
void VulkanGameOfLife::updateDesciptorSets()
{
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		{
			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->computeDescriptorSets[i], 0)
					.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->boardImageViews[i], _deviceResources->computeSampler, pvrvk::ImageLayout::e_GENERAL)));

			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_IMAGE, _deviceResources->computeDescriptorSets[i], 1)
					.setImageInfo(0,
						pvrvk::DescriptorImageInfo(_deviceResources->boardImageViews[(i + 1) % _deviceResources->swapchain->getSwapchainLength()], pvrvk::ImageLayout::e_GENERAL)));
		}

		{
			writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->graphicsDescriptorSets[i], 0)
										.setImageInfo(0,
											pvrvk::DescriptorImageInfo(_deviceResources->boardImageViews[(i + 1) % _deviceResources->swapchain->getSwapchainLength()],
												_deviceResources->graphicsSampler, pvrvk::ImageLayout::e_GENERAL)));

			writeDescSets.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->graphicsDescriptorSets[i], 1)
					.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->petriDishImageView, _deviceResources->graphicsSampler, pvrvk::ImageLayout::e_GENERAL)));
		}
	}

	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary> Creates the shader modules and associated graphics pipelines used for rendering the scene.</summary>
void VulkanGameOfLife::createPipelines()
{
	pvrvk::ShaderModule computeShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(CompShaderSrcFile)->readToEnd<uint32_t>()));
	pvrvk::ShaderModule vertexShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(VertShaderSrcFile)->readToEnd<uint32_t>()));
	pvrvk::ShaderModule fragShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(FragShaderSrcFile)->readToEnd<uint32_t>()));

	// Create Compute pipeline
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
		colorAttachmentState.setColorBlendOp(pvrvk::BlendOp::e_ADD);
		colorAttachmentState.setSrcColorBlendFactor(pvrvk::BlendFactor::e_ZERO);
		colorAttachmentState.setDstColorBlendFactor(pvrvk::BlendFactor::e_SRC_COLOR);

		createInfo.vertexShader.setShader(vertexShader);
		createInfo.fragmentShader.setShader(fragShader);
		createInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);
		createInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

		createInfo.vertexInput.clear();
		createInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);

		createInfo.colorBlend.setAttachmentState(0, colorAttachmentState);
		createInfo.pipelineLayout = _deviceResources->graphicsPipelinelayout;
		createInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		createInfo.subpass = 0;

		_deviceResources->graphicsPipeline = _deviceResources->device->createGraphicsPipeline(createInfo, _deviceResources->pipelineCache);
	}
}

/// <summary> Creates pipeline layouts and descriptor sets and associated layouts used for rendering and compute.</summary>
void VulkanGameOfLife::createResources()
{
	// Create Compute Pipeline layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descriptorSetLayoutParams;
		descriptorSetLayoutParams.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
		descriptorSetLayoutParams.setBinding(1, pvrvk::DescriptorType::e_STORAGE_IMAGE, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);

		_deviceResources->computeDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descriptorSetLayoutParams);
	}

	// Create the Graphics Pipeline layout
	{
		pvrvk::PipelineLayoutCreateInfo createInfo;
		createInfo.addDescSetLayout(_deviceResources->computeDescriptorSetLayout);
		_deviceResources->computePipelinelayout = _deviceResources->device->createPipelineLayout(createInfo);

		pvrvk::DescriptorSetLayoutCreateInfo descriptorSetLayoutParams;
		descriptorSetLayoutParams.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		descriptorSetLayoutParams.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->graphicsDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descriptorSetLayoutParams);
	}

	{
		pvrvk::PipelineLayoutCreateInfo createInfo;
		createInfo.addDescSetLayout(_deviceResources->graphicsDescriptorSetLayout);
		createInfo.addDescSetLayout(_deviceResources->graphicsDescriptorSetLayout);
		_deviceResources->graphicsPipelinelayout = _deviceResources->device->createPipelineLayout(createInfo);
	}

	// Create Sampler

	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.magFilter = pvrvk::Filter::e_NEAREST;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;

	_deviceResources->computeSampler = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;

	_deviceResources->graphicsSampler = _deviceResources->device->createSampler(samplerInfo);

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	// Update the descriptor Sets:
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->computeDescriptorSets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->computeDescriptorSetLayout);
		_deviceResources->graphicsDescriptorSets[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->graphicsDescriptorSetLayout);
	}

	updateDesciptorSets();
}

/// <summary> Record the commands used for rendering the UI elements.</summary>
void VulkanGameOfLife::recordUICmdBuffer()
{
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->uiRendererCmdBuffers[i]->begin(_deviceResources->onScreenFramebuffer[i], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
		_deviceResources->uiRenderer.beginRendering(_deviceResources->uiRendererCmdBuffers[i]);
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.endRendering();
		_deviceResources->uiRendererCmdBuffers[i]->end();
	}
}

/// <summary>Record the commands used for rendering to screen.</summary>
/// <param name="swapchainIndex"> The swapchain index signifying the frame to record commands to render.</param>
pvrvk::CommandBuffer VulkanGameOfLife::recordGraphicsCmdBuffer(const uint32_t swapchainIndex)
{
	const pvrvk::ClearValue clearValue[] = { pvrvk::ClearValue(1.0f, 1.0f, 1.0f, 1.0f) };

	pvrvk::CommandBuffer& mainCmdBuffer = _deviceResources->graphicsPrimaryCmdBuffers[swapchainIndex];
	pvrvk::SecondaryCommandBuffer& graphicsCmdBuffer = _deviceResources->graphicsCmdBuffers[swapchainIndex];

	// Recording The graphics Commandbuffer
	graphicsCmdBuffer->begin(_deviceResources->onScreenFramebuffer[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
	pvr::utils::beginCommandBufferDebugLabel(graphicsCmdBuffer, pvrvk::DebugUtilsLabel("Fragment Shader"));
	graphicsCmdBuffer->bindPipeline(_deviceResources->graphicsPipeline);
	graphicsCmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->graphicsPipelinelayout, 0, _deviceResources->graphicsDescriptorSets[currentFrameId]);
	graphicsCmdBuffer->draw(0, 3);
	pvr::utils::endCommandBufferDebugLabel(graphicsCmdBuffer);
	graphicsCmdBuffer->end();

	mainCmdBuffer->begin();
	mainCmdBuffer->beginRenderPass(_deviceResources->onScreenFramebuffer[swapchainIndex], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, clearValue, ARRAY_SIZE(clearValue));
	mainCmdBuffer->executeCommands(graphicsCmdBuffer);
	mainCmdBuffer->executeCommands(_deviceResources->uiRendererCmdBuffers[swapchainIndex]);
	mainCmdBuffer->endRenderPass();
	mainCmdBuffer->end();

	return mainCmdBuffer;
}

/// <summary>Record the commands used for computing next state of Game of life.</summary>
pvrvk::CommandBuffer VulkanGameOfLife::recordComputeCmdBuffer()
{
	pvrvk::SecondaryCommandBuffer& computeCmdBuffer = _deviceResources->computeCmdBuffers[currentFrameId];
	pvrvk::CommandBuffer& mainCmdBuffer = _deviceResources->computePrimaryCmdBuffers[currentFrameId];

	// Recording the Compute Commandbuffer
	computeCmdBuffer->reset();
	computeCmdBuffer->begin();
	pvr::utils::beginCommandBufferDebugLabel(computeCmdBuffer, pvrvk::DebugUtilsLabel("Compute Stage"));
	computeCmdBuffer->bindPipeline(_deviceResources->computePipeline);
	computeCmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_COMPUTE, _deviceResources->computePipelinelayout, 0, _deviceResources->computeDescriptorSets[currentFrameId]);
	computeCmdBuffer->dispatch(boardWidth / 8, boardHeight / 4, 1);

	computeCmdBuffer->end();

	mainCmdBuffer->begin();
	mainCmdBuffer->executeCommands(computeCmdBuffer);
	mainCmdBuffer->end();

	return mainCmdBuffer;
}

/// <summary>Record the commands used for computing next state of Game of life.</summary>
/// <param name="submitCmdBuffer">The Command buffer used to submit compute work.</param>
void VulkanGameOfLife::submitComputeWork(pvrvk::CommandBuffer submitCmdBuffer)
{
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	// Set the semaphores to be waited on and signalled
	pvrvk::PipelineStageFlags computePipeWaitStageFlags[] = { pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT, pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT };
	pvrvk::Semaphore computeWaitSemaphores[] = { _deviceResources->computeToComputeSemaphores[previousFrameId], _deviceResources->renderToComputeSemaphore[renderComputeSyncId] };
	pvrvk::Semaphore computeSignalSemaphores[] = { _deviceResources->computeToComputeSemaphores[currentFrameId], _deviceResources->computeToRenderSemaphore[currentFrameId] };

	// Submit
	pvrvk::SubmitInfo computeSubmitInfo;
	computeSubmitInfo.commandBuffers = &submitCmdBuffer;
	computeSubmitInfo.numCommandBuffers = 1;
	computeSubmitInfo.waitSemaphores = computeWaitSemaphores;
	computeSubmitInfo.signalSemaphores = computeSignalSemaphores;
	computeSubmitInfo.waitDstStageMask = computePipeWaitStageFlags;

	computeSubmitInfo.numWaitSemaphores = (stepCount == 0 ? 0 : 1);
	computeSubmitInfo.numSignalSemaphores = 2;

	if (stepCount >= swapchainLength) { computeSubmitInfo.numWaitSemaphores++; }

	_deviceResources->queues[computeQueueIndex]->submit(&computeSubmitInfo, 1, _deviceResources->computeFences[currentFrameId]);
}

/// <summary>Submit the commands used for rendering to screen </summary>
/// <param name="submitCmdBuffer"> The Command buffer used to submit graphics work.</param>
void VulkanGameOfLife::submitGraphicsWork(pvrvk::CommandBuffer submitCmdBuffer)
{
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// Set the semaphores to be waited on and signalled
	pvrvk::PipelineStageFlags pipeWaitStageFlags[] = { pvrvk::PipelineStageFlags::e_FRAGMENT_SHADER_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT };
	pvrvk::Semaphore graphicsWaitSemaphores[] = { _deviceResources->computeToRenderSemaphore[currentFrameId], _deviceResources->imageAcquiredSemaphores[currentFrameId] };
	pvrvk::Semaphore graphicsSignalSemaphores[] = { _deviceResources->renderToComputeSemaphore[currentFrameId], _deviceResources->presentationSemaphores[currentFrameId] };

	// Submit
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &submitCmdBuffer;
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = graphicsWaitSemaphores;
	submitInfo.numWaitSemaphores = 2;
	submitInfo.signalSemaphores = graphicsSignalSemaphores;
	submitInfo.numSignalSemaphores = 2;
	submitInfo.waitDstStageMask = pipeWaitStageFlags;

	_deviceResources->queues[graphicsQueueIndex]->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanGameOfLife::initApplication()
{
	// Setting VsyncMode to FIFO
	setBackBufferColorspace(pvr::ColorSpace::lRGB);

	currentFrameId = 0;
	previousFrameId = 0;
	renderComputeSyncId = 0;

	graphicsQueueIndex = 0;
	computeQueueIndex = 1;
	setZoomLevel(1);
	currBoardConfig = 0;

	boardConfigUI = "\nBoard Config : ";
	boardConfigUI += boardConfigs[currBoardConfig];

	this->setDepthBitsPerPixel(0);
	this->setStencilBitsPerPixel(0);

	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns> Result::Success if no error occurred.</returns>
pvr::Result VulkanGameOfLife::initView()
{
	// Initialise device resources.
	_deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources());
	_deviceResources->instance = pvr::utils::createInstance(this->getApplicationName());

	// Query the number of physical devices available. if none, exit.
	if (_deviceResources->instance->getNumPhysicalDevices() == 0)
	{
		setExitMessage("Unable not find a compatible Vulkan physical device.");
		return pvr::Result::UnknownError;
	}
	// Create surface
	pvrvk::Surface surface =
		pvr::utils::createSurface(_deviceResources->instance, _deviceResources->instance->getPhysicalDevice(0), this->getWindow(), this->getDisplay(), this->getConnection());

	// Create a default set of debug utils messengers or debug callbacks using either VK_EXT_debug_utils or VK_EXT_debug_report respectively
	_deviceResources->debugUtilsCallbacks = pvr::utils::createDebugUtilsCallbacks(_deviceResources->instance);

	pvr::utils::QueuePopulateInfo queueCreateInfos[] = {
		{ pvrvk::QueueFlags::e_GRAPHICS_BIT, surface }, // Queue 0 for Graphics
		{ pvrvk::QueueFlags::e_COMPUTE_BIT } // Queue 1 For Compute
	};

	pvr::utils::QueueAccessInfo queueAccessInfos[2];
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueCreateInfos, 2, queueAccessInfos);

	_deviceResources->queues[0] = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);

	// In the future we may want to improve our flexibility with regards to making use of multiple queues but for now to support multi queue the queue must support
	// Graphics + Compute + WSI support.
	// Other multi queue approaches may be possible i.e. making use of additional queues which do not support graphics/WSI
	useMultiQueue = false;
	if (queueAccessInfos[1].familyId != -1 && queueAccessInfos[1].queueId != -1)
	{
		useMultiQueue = true;
		Log(LogLevel::Information, "Multiple queues support e_GRAPHICS_BIT + e_COMPUTE_BIT + WSI. These queues will be used to ping-pong work each frame");

		_deviceResources->queues[1] = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);
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
	// Create the Swapchain, its renderpass, attachments and framebuffers. Will support MSAA if enabled through command line.
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage).enableDepthBuffer(false));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	_deviceResources->cmdPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queues[0]->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	_deviceResources->computeCmdPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queues[1]->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool =
		_deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo(10).addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_IMAGE, 16));

	// Create per frame Resources.
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->graphicsPrimaryCmdBuffers[i] = _deviceResources->cmdPool->allocateCommandBuffer();
		_deviceResources->computePrimaryCmdBuffers[i] = _deviceResources->computeCmdPool->allocateCommandBuffer();

		_deviceResources->uiRendererCmdBuffers[i] = _deviceResources->cmdPool->allocateSecondaryCommandBuffer();
		_deviceResources->graphicsCmdBuffers[i] = _deviceResources->cmdPool->allocateSecondaryCommandBuffer();
		_deviceResources->computeCmdBuffers[i] = _deviceResources->computeCmdPool->allocateSecondaryCommandBuffer();

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();

		_deviceResources->computeToComputeSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->computeToRenderSemaphore[i] = _deviceResources->device->createSemaphore();
		_deviceResources->renderToComputeSemaphore[i] = _deviceResources->device->createSemaphore();

		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
		_deviceResources->computeFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	board.resize(static_cast<size_t>(boardWidth * boardHeight));
	generateBoardData();

	// Load Textures used by in the demo
	_deviceResources->graphicsPrimaryCmdBuffers[0]->begin();
	createPetriDishEffect(_deviceResources->graphicsPrimaryCmdBuffers[0]);
	generateTextures(_deviceResources->graphicsPrimaryCmdBuffers[0]);
	_deviceResources->graphicsPrimaryCmdBuffers[0]->end();

	// Submit the image upload command buffer
	pvrvk::SubmitInfo submit;
	submit.commandBuffers = &_deviceResources->graphicsPrimaryCmdBuffers[0];
	submit.numCommandBuffers = 1;
	_deviceResources->queues[0]->submit(&submit, 1);
	_deviceResources->queues[0]->waitIdle();

	_deviceResources->graphicsPrimaryCmdBuffers[0]->reset(pvrvk::CommandBufferResetFlags::e_RELEASE_RESOURCES_BIT);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	createResources();
	createPipelines();

	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->cmdPool, _deviceResources->queues[0]);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Game of Life");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();

	_deviceResources->uiRenderer.getDefaultControls()->setText("Action 1: Reset Simulation\n"
															   "Up / Down: Zoom In/Out\n"
															   "Left / Right: Change Board Config");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	recordUICmdBuffer();

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanGameOfLife::renderFrame()
{
	// Do Compute Work
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	_deviceResources->computeFences[currentFrameId]->wait();
	_deviceResources->computeFences[currentFrameId]->reset();

	submitComputeWork(recordComputeCmdBuffer());

	std::string uiDescription = "Generation: " + std::to_string(generation);
	uiDescription += boardConfigUI;
	uiDescription += zoomRatioUI;

	_deviceResources->uiRenderer.getDefaultDescription()->setText(uiDescription);
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();

	// Do Graphics work
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[currentFrameId]);
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	submitGraphicsWork(recordGraphicsCmdBuffer(swapchainIndex));

	// Take screenshot if the command-line argument is passed
	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queues[graphicsQueueIndex], _deviceResources->cmdPool, _deviceResources->swapchain, swapchainIndex,
			this->getScreenshotFileName(), _deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[currentFrameId];
	presentInfo.numSwapchains = 1;

	_deviceResources->queues[graphicsQueueIndex]->present(presentInfo);

	previousFrameId = currentFrameId;
	currentFrameId = (currentFrameId + 1) % swapchainLength;

	if (stepCount < swapchainLength) { stepCount++; }
	else
	{
		renderComputeSyncId = (renderComputeSyncId + 1) % swapchainLength;
	}

	generation++;
	return pvr::Result::Success;
}

/// <summary>Handles user input and updates live variables accordingly.</summary>
/// <param name="key">Input key to handle</param>
void VulkanGameOfLife::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::NONE: break;

	// Switch between Board Configurations.
	case pvr::SimplifiedInput::Left:
	case pvr::SimplifiedInput::Right:
		currBoardConfig += (key == pvr::SimplifiedInput::Right ? 1 : -1);
		if (currBoardConfig >= BoardConfig::NumBoards) { currBoardConfig = 0; }
		else if (currBoardConfig < 0)
		{
			currBoardConfig = BoardConfig::NumBoards - 1;
		}

		boardConfigUI = "\nBoard Config : ";
		boardConfigUI += boardConfigs[currBoardConfig];

		refreshBoard(true);
		break;

	// Zoom In or Out of the Board.
	case pvr::SimplifiedInput::Up:
	case pvr::SimplifiedInput::Down:
		setZoomLevel(zoomLevel + (key == pvr::SimplifiedInput::Up ? 1 : -1));
		refreshBoard(true);
		break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;

	// Refresh the board.
	case pvr::SimplifiedInput::Action1: refreshBoard(); break;
	case pvr::SimplifiedInput::Action2: break;
	case pvr::SimplifiedInput::Action3: break;
	default: break;
	}
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanGameOfLife::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred.</returns>.
pvr::Result VulkanGameOfLife::quitApplication() { return pvr::Result::Success; }

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::unique_ptr<pvr::Shell>(new VulkanGameOfLife()); }
