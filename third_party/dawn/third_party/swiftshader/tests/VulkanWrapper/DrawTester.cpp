// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "DrawTester.hpp"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

DrawTester::DrawTester(Multisample multisample)
    : multisample(multisample == Multisample::True)
{
}

DrawTester::~DrawTester()
{
	device.freeCommandBuffers(commandPool, commandBuffers);

	device.destroyDescriptorPool(descriptorPool);
	for(auto &sampler : samplers)
	{
		device.destroySampler(sampler, nullptr);
	}
	images.clear();
	device.destroyCommandPool(commandPool, nullptr);

	for(auto &fence : waitFences)
	{
		device.destroyFence(fence, nullptr);
	}

	device.destroySemaphore(renderCompleteSemaphore, nullptr);
	device.destroySemaphore(presentCompleteSemaphore, nullptr);

	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipelineLayout, nullptr);
	device.destroyDescriptorSetLayout(descriptorSetLayout);

	device.freeMemory(vertices.memory, nullptr);
	device.destroyBuffer(vertices.buffer, nullptr);

	for(auto &framebuffer : framebuffers)
	{
		framebuffer.reset();
	}

	device.destroyRenderPass(renderPass, nullptr);

	swapchain.reset();
	window.reset();
}

void DrawTester::initialize()
{
	VulkanTester::initialize();

	window.reset(new Window(instance, windowSize));
	swapchain.reset(new Swapchain(physicalDevice, device, *window));

	renderPass = createRenderPass(swapchain->colorFormat);
	createFramebuffers(renderPass);

	prepareVertices();

	pipeline = createGraphicsPipeline(renderPass);

	createSynchronizationPrimitives();

	createCommandBuffers(renderPass);
}

void DrawTester::renderFrame()
{
	swapchain->acquireNextImage(presentCompleteSemaphore, currentFrameBuffer);

	device.waitForFences(1, &waitFences[currentFrameBuffer], VK_TRUE, UINT64_MAX);
	device.resetFences(1, &waitFences[currentFrameBuffer]);

	vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::SubmitInfo submitInfo;
	submitInfo.pWaitDstStageMask = &waitStageMask;
	submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderCompleteSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrameBuffer];
	submitInfo.commandBufferCount = 1;

	queue.submit(1, &submitInfo, waitFences[currentFrameBuffer]);

	swapchain->queuePresent(queue, currentFrameBuffer, renderCompleteSemaphore);
}

void DrawTester::show()
{
	window->show();
}

vk::RenderPass DrawTester::createRenderPass(vk::Format colorFormat)
{
	std::vector<vk::AttachmentDescription> attachments(multisample ? 2 : 1);

	if(multisample)
	{
		// Color attachment
		attachments[0].format = colorFormat;
		attachments[0].samples = vk::SampleCountFlagBits::e4;
		attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[0].initialLayout = vk::ImageLayout::eUndefined;
		attachments[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

		// Resolve attachment
		attachments[1].format = colorFormat;
		attachments[1].samples = vk::SampleCountFlagBits::e1;
		attachments[1].loadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[1].initialLayout = vk::ImageLayout::eUndefined;
		attachments[1].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	}
	else
	{
		attachments[0].format = colorFormat;
		attachments[0].samples = vk::SampleCountFlagBits::e1;
		attachments[0].loadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[0].initialLayout = vk::ImageLayout::eUndefined;
		attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
	}

	vk::AttachmentReference attachment0;
	attachment0.attachment = 0;
	attachment0.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference attachment1;
	attachment1.attachment = 1;
	attachment1.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpassDescription;
	subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pResolveAttachments = multisample ? &attachment1 : nullptr;
	subpassDescription.pColorAttachments = &attachment0;

	std::array<vk::SubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
	dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
	dependencies[0].dstAccessMask = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite;
	dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
	dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	dependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
	dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	return device.createRenderPass(renderPassInfo);
}

void DrawTester::createFramebuffers(vk::RenderPass renderPass)
{
	framebuffers.resize(swapchain->imageCount());

	for(size_t i = 0; i < framebuffers.size(); i++)
	{
		framebuffers[i].reset(new Framebuffer(device, physicalDevice, swapchain->getImageView(i), swapchain->colorFormat, renderPass, swapchain->getExtent(), multisample));
	}
}

void DrawTester::prepareVertices()
{
	hooks.createVertexBuffers(*this);
}

vk::Pipeline DrawTester::createGraphicsPipeline(vk::RenderPass renderPass)
{
	auto setLayoutBindings = hooks.createDescriptorSetLayout(*this);

	std::vector<vk::DescriptorSetLayout> setLayouts;
	if(!setLayoutBindings.empty())
	{
		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		layoutInfo.pBindings = setLayoutBindings.data();
		descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

		setLayouts.push_back(descriptorSetLayout);
	}

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
	pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
	inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

	vk::PipelineRasterizationStateCreateInfo rasterizationState;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = vk::PolygonMode::eFill;
	rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
	rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	vk::PipelineColorBlendAttachmentState blendAttachmentState;
	blendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	blendAttachmentState.blendEnable = VK_FALSE;
	vk::PipelineColorBlendStateCreateInfo colorBlendState;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &blendAttachmentState;

	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	std::vector<vk::DynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(vk::DynamicState::eViewport);
	dynamicStateEnables.push_back(vk::DynamicState::eScissor);
	vk::PipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	vk::PipelineDepthStencilStateCreateInfo depthStencilState;
	depthStencilState.depthTestEnable = VK_FALSE;
	depthStencilState.depthWriteEnable = VK_FALSE;
	depthStencilState.depthCompareOp = vk::CompareOp::eLessOrEqual;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.back.failOp = vk::StencilOp::eKeep;
	depthStencilState.back.passOp = vk::StencilOp::eKeep;
	depthStencilState.back.compareOp = vk::CompareOp::eAlways;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = depthStencilState.back;

	vk::PipelineMultisampleStateCreateInfo multisampleState;
	multisampleState.rasterizationSamples = multisample ? vk::SampleCountFlagBits::e4 : vk::SampleCountFlagBits::e1;
	multisampleState.pSampleMask = nullptr;

	vk::ShaderModule vertexModule = hooks.createVertexShader(*this);
	vk::ShaderModule fragmentModule = hooks.createFragmentShader(*this);

	assert(vertexModule);    // TODO: if nullptr, use a default
	assert(fragmentModule);  // TODO: if nullptr, use a default

	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

	shaderStages[0].module = vertexModule;
	shaderStages[0].stage = vk::ShaderStageFlagBits::eVertex;
	shaderStages[0].pName = "main";

	shaderStages[1].module = fragmentModule;
	shaderStages[1].stage = vk::ShaderStageFlagBits::eFragment;
	shaderStages[1].pName = "main";

	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &vertices.inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	auto pipeline = device.createGraphicsPipeline(nullptr, pipelineCreateInfo).value;

	device.destroyShaderModule(fragmentModule);
	device.destroyShaderModule(vertexModule);

	return pipeline;
}

void DrawTester::createSynchronizationPrimitives()
{
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	presentCompleteSemaphore = device.createSemaphore(semaphoreCreateInfo);
	renderCompleteSemaphore = device.createSemaphore(semaphoreCreateInfo);

	vk::FenceCreateInfo fenceCreateInfo;
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
	waitFences.resize(swapchain->imageCount());
	for(auto &fence : waitFences)
	{
		fence = device.createFence(fenceCreateInfo);
	}
}

void DrawTester::createCommandBuffers(vk::RenderPass renderPass)
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
	commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPool = device.createCommandPool(commandPoolCreateInfo);

	std::vector<vk::DescriptorSet> descriptorSets;
	if(descriptorSetLayout)
	{
		std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
		poolSizes[0].type = vk::DescriptorType::eCombinedImageSampler;
		poolSizes[0].descriptorCount = 1;

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;

		descriptorPool = device.createDescriptorPool(poolInfo);

		std::vector<vk::DescriptorSetLayout> layouts(1, descriptorSetLayout);
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets = device.allocateDescriptorSets(allocInfo);

		hooks.updateDescriptorSet(*this, commandPool, descriptorSets[0]);
	}

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(swapchain->imageCount());
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;

	commandBuffers = device.allocateCommandBuffers(commandBufferAllocateInfo);

	for(size_t i = 0; i < commandBuffers.size(); i++)
	{
		vk::CommandBufferBeginInfo commandBufferBeginInfo;
		commandBuffers[i].begin(commandBufferBeginInfo);

		vk::ClearValue clearValues[1];
		clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{ 0.5f, 0.5f, 0.5f, 1.0f });

		vk::RenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.framebuffer = framebuffers[i]->getFramebuffer();
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = windowSize;
		renderPassBeginInfo.clearValueCount = ARRAY_SIZE(clearValues);
		renderPassBeginInfo.pClearValues = clearValues;
		commandBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		// Set dynamic state
		vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(windowSize.width), static_cast<float>(windowSize.height), 0.0f, 1.0f);
		commandBuffers[i].setViewport(0, 1, &viewport);

		vk::Rect2D scissor(vk::Offset2D(0, 0), windowSize);
		commandBuffers[i].setScissor(0, 1, &scissor);

		if(!descriptorSets.empty())
		{
			commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[0], 0, nullptr);
		}

		// Draw
		if(vertices.numVertices > 0)
		{
			commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
			VULKAN_HPP_NAMESPACE::DeviceSize offset = 0;
			commandBuffers[i].bindVertexBuffers(0, 1, &vertices.buffer, &offset);
			commandBuffers[i].draw(vertices.numVertices, 1, 0, 0);
		}

		commandBuffers[i].endRenderPass();
		commandBuffers[i].end();
	}
}

void DrawTester::addVertexBuffer(void *vertexBufferData, size_t vertexBufferDataSize, size_t vertexSize, std::vector<vk::VertexInputAttributeDescription> inputAttributes)
{
	assert(!vertices.buffer);  // For now, only support adding once

	vk::BufferCreateInfo vertexBufferInfo;
	vertexBufferInfo.size = vertexBufferDataSize;
	vertexBufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
	vertices.buffer = device.createBuffer(vertexBufferInfo);

	vk::MemoryAllocateInfo memoryAllocateInfo;
	vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(vertices.buffer);
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = Util::getMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	vertices.memory = device.allocateMemory(memoryAllocateInfo);

	void *data = device.mapMemory(vertices.memory, 0, VK_WHOLE_SIZE);
	memcpy(data, vertexBufferData, vertexBufferDataSize);
	device.unmapMemory(vertices.memory);
	device.bindBufferMemory(vertices.buffer, vertices.memory, 0);

	vertices.inputBinding.binding = 0;
	vertices.inputBinding.stride = static_cast<uint32_t>(vertexSize);
	vertices.inputBinding.inputRate = vk::VertexInputRate::eVertex;

	vertices.inputAttributes = std::move(inputAttributes);

	vertices.inputState.vertexBindingDescriptionCount = 1;
	vertices.inputState.pVertexBindingDescriptions = &vertices.inputBinding;
	vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.inputAttributes.size());
	vertices.inputState.pVertexAttributeDescriptions = vertices.inputAttributes.data();

	// Note that we assume data is tightly packed
	vertices.numVertices = static_cast<uint32_t>(vertexBufferDataSize / vertexSize);
}

vk::ShaderModule DrawTester::createShaderModule(const char *glslSource, EShLanguage glslLanguage)
{
	auto spirv = Util::compileGLSLtoSPIRV(glslSource, glslLanguage);

	vk::ShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.codeSize = spirv.size() * sizeof(uint32_t);
	moduleCreateInfo.pCode = (uint32_t *)spirv.data();

	return device.createShaderModule(moduleCreateInfo);
}
