/*!
\brief Contains Vulkan-specific utilities to facilitate Physically Based Rendering tasks, such as generating irradiance maps and BRDF lookup tables.
\file PVRUtils/Vulkan/PBRUtilsVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "PBRUtilsVk.h"
#include "PVRUtils/StructuredMemory.h"
#include "PVRUtils/Vulkan/HelperVk.h"
#include "PVRUtils/Vulkan/PBRUtilsVertShader.h"
#include "PVRUtils/Vulkan/PBRUtilsIrradianceFragShader.h"
#include "PVRUtils/Vulkan/PBRUtilsPrefilteredFragShader.h"
#include "PVRCore/textureio/TextureWriterPVR.h"

namespace pvr {
namespace utils {

pvr::Texture generateIrradianceMap(
	pvrvk::Queue queue, pvrvk::ImageView environmentMap, pvr::PixelFormat outputFormat, pvr::VariableType outputFormatType, uint32_t mapSize, uint32_t mapNumSamples)
{
	pvrvk::Device device = queue->getDevice();
	device->waitIdle();

	pvrvk::Image renderTarget;
	pvrvk::Image outputImage;

	pvrvk::CommandPool cmdPool = device->createCommandPool(pvrvk::CommandPoolCreateInfo(queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	pvrvk::DescriptorPool descPool = device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo(1));
	pvr::utils::vma::Allocator allocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(device));

	uint32_t numMipLevels = static_cast<uint32_t>(log2(static_cast<float>(mapSize)) + 1.0);

	// calculate the mip level dimensions.
	std::vector<uint32_t> mipLevelDimensions(numMipLevels);
	for (uint32_t i = 0; i < mipLevelDimensions.size(); ++i) { mipLevelDimensions[i] = static_cast<uint32_t>(pow(2, numMipLevels - i - 1)); }

	pvrvk::CommandBuffer cmdBuffer = cmdPool->allocateCommandBuffer();

	// create descriptor set layout
	pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
	descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	descSetLayoutInfo.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
	pvrvk::DescriptorSetLayout setLayout = cmdPool->getDevice()->createDescriptorSetLayout(descSetLayoutInfo);

	// Create the descriptor set
	pvrvk::DescriptorSet descSet = descPool->allocateDescriptorSet(setLayout);

	pvrvk::Sampler sampler = device->createSampler(pvrvk::SamplerCreateInfo(pvrvk::Filter::e_LINEAR, pvrvk::Filter::e_LINEAR, pvrvk::SamplerMipmapMode::e_LINEAR,
		pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE, pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE, pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE));

	pvr::utils::StructuredBufferView uboView;
	pvr::utils::StructuredMemoryDescription viewDesc;
	viewDesc.addElement("rotateMtx", pvr::GpuDatatypes::mat3x3);
	uboView.initDynamic(viewDesc, 6 /*num faces on cube*/, pvr::BufferUsageFlags::UniformBuffer,
		cmdPool->getDevice()->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

	// Create the uniform buffer storing the rotation matrix for the cube map view direction
	pvrvk::Buffer uboBuffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(uboView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, allocator);
	uboView.pointToMappedMemory(uboBuffer->getDeviceMemory()->getMappedData());

	const glm::mat3 cubeView[] = {
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.f))), // +X
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(0.0f, 1.0f, 0.f))), // -X
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(1.0f, .0f, 0.f))), // +Y
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(1.0f, .0f, 0.f))), // -Y
		glm::mat3(glm::scale(glm::vec3(1.0f, -1.0f, 1.f))), // +Z
		glm::mat3(glm::scale(glm::vec3(-1.0f, -1.0f, -1.f))), // -Z
	};

	uboView.getElement(0, 0, 0).setValue(cubeView[0]);
	uboView.getElement(0, 0, 1).setValue(cubeView[1]);
	uboView.getElement(0, 0, 2).setValue(cubeView[2]);
	uboView.getElement(0, 0, 3).setValue(cubeView[3]);
	uboView.getElement(0, 0, 4).setValue(cubeView[4]);
	uboView.getElement(0, 0, 5).setValue(cubeView[5]);

	if (uint32_t(uboBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0) { uboBuffer->getDeviceMemory()->flushRange(); }

	const pvrvk::ClearValue clearValue = pvrvk::ClearValue(1.f, 1.f, 0.f, 1.0f);

	const pvrvk::Format format = pvrvk::Format::e_R16G16B16A16_SFLOAT;
	const pvrvk::Format outputVkFormat = pvr::utils::convertToPVRVkPixelFormat(outputFormat, pvr::ColorSpace::lRGB, outputFormatType);

	if (outputVkFormat == pvrvk::Format::e_UNDEFINED)
	{ throw InvalidArgumentError("format,type", "The provided PixelFormat and VariableType do not map to a valid Vulkan format"); }

	const uint32_t outputFormatStride = outputFormat.getBitsPerPixel() / 8;

	pvrvk::ImageView imageView;
	pvrvk::RenderPass renderpass;
	pvrvk::Framebuffer fbo;

	const uint32_t bufferSize = mipLevelDimensions[0] * mipLevelDimensions[0] * 6 * outputFormatStride * numMipLevels;

	// Create a buffer to use as the final destination for the image data which will be saved to disk
	pvrvk::Buffer imageDataBuffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(bufferSize), pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, allocator);

	pvrvk::WriteDescriptorSet descSetUpdate[] = {
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descSet, 0),
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, descSet, 1),
	};
	descSetUpdate[0].setImageInfo(0, pvrvk::DescriptorImageInfo(environmentMap, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL)); // environment map
	descSetUpdate[1].setBufferInfo(0, pvrvk::DescriptorBufferInfo(uboBuffer, 0, uboView.getDynamicSliceSize()));

	device->updateDescriptorSets(descSetUpdate, ARRAY_SIZE(descSetUpdate), nullptr, 0);

	// Create an image to use as the destination for per level, per face color attachment writes
	renderTarget = pvr::utils::createImage(device,
		pvrvk::ImageCreateInfo(
			pvrvk::ImageType::e_2D, format, pvrvk::Extent3D(mapSize, mapSize, 1), pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, allocator);

	// Create an image view for the render target so that it can be used a framebuffer attachment
	imageView = device->createImageView(pvrvk::ImageViewCreateInfo(renderTarget));

	// Create an image to use as the destination for per level, per face transfer operations which may involve format translation
	outputImage = pvr::utils::createImage(device,
		pvrvk::ImageCreateInfo(
			pvrvk::ImageType::e_2D, outputVkFormat, pvrvk::Extent3D(mapSize, mapSize, 1), pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, allocator);

	// create the renderpass
	pvrvk::RenderPassCreateInfo rpInfo;
	rpInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(
			format, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE));

	pvrvk::SubpassDescription subpassDesc;
	subpassDesc.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

	rpInfo.setSubpass(0, subpassDesc);
	renderpass = device->createRenderPass(rpInfo);

	// Create the fbo
	pvrvk::FramebufferCreateInfo fboInfo;
	fboInfo.setAttachment(0, imageView);
	fboInfo.setDimensions(mapSize, mapSize);
	fboInfo.setRenderPass(renderpass);
	fbo = device->createFramebuffer(fboInfo);

	// create graphics pipeline.
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.depthStencil.enableAllStates(false);

	pipelineInfo.vertexShader = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(spv_PBRUtilsVertShader, sizeof(spv_PBRUtilsVertShader) / sizeof(spv_PBRUtilsVertShader[0])));
	pipelineInfo.fragmentShader = device->createShaderModule(
		pvrvk::ShaderModuleCreateInfo(spv_PBRUtilsIrradianceFragShader, sizeof(spv_PBRUtilsIrradianceFragShader) / sizeof(spv_PBRUtilsIrradianceFragShader[0])));

	uint32_t numSamplesPerDir = uint32_t(std::max(sqrtf(float(mapNumSamples)), 1.f));

	pipelineInfo.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &numSamplesPerDir, sizeof(uint32_t)));

	// depth stencil state
	pipelineInfo.depthStencil.enableDepthWrite(false);
	pipelineInfo.depthStencil.enableDepthTest(false);

	// rasterizer state
	pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);
	pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
	// blend state
	pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	// input assembler
	pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
	pipelineInfo.renderPass = renderpass;

	// vertex attributes and bindings
	pipelineInfo.vertexInput.clear();
	pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(), pvrvk::Rect2D());

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(descSet->getDescriptorSetLayout());

	pipelineInfo.pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	pipelineInfo.dynamicStates.setDynamicState(pvrvk::DynamicState::e_VIEWPORT, true);
	pipelineInfo.dynamicStates.setDynamicState(pvrvk::DynamicState::e_SCISSOR, true);

	pipeline = device->createGraphicsPipeline(pipelineInfo);

	// record commands
	cmdBuffer->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("pvr::utils::generateIrradianceMap"));

	pvr::utils::setImageLayout(outputImage, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, cmdBuffer);

	cmdBuffer->bindPipeline(pipeline);

	VkDeviceSize bufferOffset = 0;
	for (uint32_t mipLevel = 0; mipLevel < numMipLevels; ++mipLevel)
	{
		uint32_t dim = mipLevelDimensions[mipLevel];
		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("Cubemap level"));
		cmdBuffer->setViewport(pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(dim), static_cast<float>(dim)));
		const pvrvk::Rect2D scissor(pvrvk::Offset2D(), pvrvk::Extent2D(dim, dim));
		cmdBuffer->setScissor(0, 1, &scissor);

		// draw each face
		for (uint32_t j = 0; j < 6; ++j)
		{
			uint32_t offset = uboView.getDynamicSliceOffset(j);
			cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipeline->getPipelineLayout(), 0, descSet, &offset, 1);

			// Render to the renderTarget
			cmdBuffer->beginRenderPass(fbo, true, &clearValue, 1);
			cmdBuffer->draw(0, 6);
			cmdBuffer->endRenderPass();

			// Add a pipeline barrier for pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT -> pvrvk::AccessFlags::e_TRANSFER_READ_BIT
			{
				pvrvk::MemoryBarrierSet barriers;
				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_TRANSFER_READ_BIT, renderTarget,
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_TRANSFER_BIT, barriers);
			}

			pvrvk::Offset3D offsets[2] = { { 0, 0, 0 }, { static_cast<int32_t>(dim), static_cast<int32_t>(dim), 1 } };

			// Copy the renderTarget to outputImage performing any format translations as they are required
			const pvrvk::ImageBlit imageCopyRegions(pvrvk::ImageSubresourceLayers(), offsets, pvrvk::ImageSubresourceLayers(), offsets);
			cmdBuffer->blitImage(
				renderTarget, outputImage, &imageCopyRegions, 1, pvrvk::Filter::e_NEAREST, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);

			// Add a pipeline barrier for the output image of pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT -> pvrvk::AccessFlags::e_TRANSFER_READ_BIT
			{
				pvrvk::MemoryBarrierSet barriers;
				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_TRANSFER_READ_BIT, outputImage,
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				cmdBuffer->pipelineBarrier(
					pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_TRANSFER_BIT | pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, barriers);
			}

			// Copy the image data to the image data buffer ready to be saved to disk
			const pvrvk::BufferImageCopy regions(bufferOffset, 0, 0, pvrvk::ImageSubresourceLayers(), pvrvk::Offset3D(), pvrvk::Extent3D(dim, dim, 1));
			cmdBuffer->copyImageToBuffer(outputImage, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, imageDataBuffer, &regions, 1);

			// Add a pipeline barrier for the output image of pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT -> pvrvk::AccessFlags::e_HOST_READ_BIT
			{
				pvrvk::MemoryBarrierSet barriers;
				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_HOST_READ_BIT, outputImage,
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_HOST_BIT, barriers);
			}

			bufferOffset += outputFormatStride * dim * dim;
		}
		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	}
	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);

	cmdBuffer->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cmdBuffer;
	submitInfo.numCommandBuffers = 1;

	pvrvk::Fence fence = device->createFence();
	queue->submit(submitInfo, fence);
	fence->wait();

	// Ensure that if the image data buffer memory backing does not have pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT that the data is visible to the host
	if (uint32_t(imageDataBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ imageDataBuffer->getDeviceMemory()->invalidateRange(); } // Get a pointer to the diffuse irradiance image data from the buffer and save it in to the file.
	void* data;
	data = imageDataBuffer->getDeviceMemory()->getMappedData();

	// Save the image to disk
	pvr::TextureHeader texHeader;
	texHeader.setChannelType(outputFormatType);
	texHeader.setColorSpace(pvr::ColorSpace::lRGB);
	texHeader.setDepth(1);
	texHeader.setWidth(mapSize);
	texHeader.setHeight(mapSize);
	texHeader.setNumMipMapLevels(numMipLevels);
	texHeader.setNumFaces(6);
	texHeader.setNumArrayMembers(1);
	texHeader.setPixelFormat(outputFormat);

	return Texture(texHeader, static_cast<const unsigned char*>(data));
}

Texture generatePreFilteredMapMipmapStyle(pvrvk::Queue queue, pvrvk::ImageView environmentMap, pvr::PixelFormat outputFormat, pvr::VariableType outputFormatType, uint32_t mapSize,
	bool zeroRoughnessIsExternal, int numMipLevelsToDiscard, uint32_t mapNumSamples)
{
	const uint32_t numFaces = 6;

	// Rendered output image and image view
	pvrvk::Image renderTarget;
	pvrvk::ImageView imageView;

	// Final format converted image
	pvrvk::Image outputImage;

	// Discard the last two mipmaps. From our experimentation keeping the last miplevel 4x4 avoids blocky texel artifacts for materials with roughness values of 1.0.

	// calculate number of mip map levels
	const uint32_t numMipLevels = static_cast<uint32_t>(log2(static_cast<float>(mapSize)) + 1.0f - numMipLevelsToDiscard); // prefilterMap

	// Calculate the mipmap level dimensions
	std::vector<uint32_t> mipLevelDimensions(numMipLevels);
	for (size_t i = 0; i < mipLevelDimensions.size(); ++i) { mipLevelDimensions[i] = static_cast<uint32_t>(pow(2, numMipLevels + numMipLevelsToDiscard - i - 1)); }

	pvrvk::Device device = queue->getDevice();
	device->waitIdle();

	pvrvk::CommandPool cmdPool = device->createCommandPool(pvrvk::CommandPoolCreateInfo(queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	pvrvk::DescriptorPool descPool = device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo(1));
	pvr::utils::vma::Allocator allocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(device));

	pvrvk::CommandBuffer cmdBuffer = cmdPool->allocateCommandBuffer();

	// Create descriptor set layout
	pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;
	descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
	descSetLayout.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
	pvrvk::DescriptorSetLayout setLayout = device->createDescriptorSetLayout(descSetLayout);

	// Create the descriptor set
	pvrvk::DescriptorSet descSet = descPool->allocateDescriptorSet(setLayout);

	// Create the uniform buffer storing the rotation matrix for the cube map view direction
	pvr::utils::StructuredBufferView uboView;
	pvr::utils::StructuredMemoryDescription viewDesc;
	viewDesc.addElement("rotateMtx", pvr::GpuDatatypes::mat3x3);
	uboView.initDynamic(viewDesc, numFaces, pvr::BufferUsageFlags::UniformBuffer, device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

	pvrvk::Buffer uboBuffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(uboView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, allocator);
	uboView.pointToMappedMemory(uboBuffer->getDeviceMemory()->getMappedData());

	const glm::mat3 cubeView[numFaces] = {
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.f))), // +X
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(0.0f, 1.0f, 0.f))), // -X
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(90.f), glm::vec3(1.0f, .0f, 0.f))), // +Y
		glm::mat3(glm::scale(glm::vec3(1.f, -1.f, 1.f)) * glm::rotate(glm::radians(-90.f), glm::vec3(1.0f, .0f, 0.f))), // -Y
		glm::mat3(glm::scale(glm::vec3(1.0f, -1.0f, 1.f))), // +Z
		glm::mat3(glm::scale(glm::vec3(-1.0f, -1.0f, -1.f))), // -Z
	};

	uboView.getElement(0, 0, 0).setValue(cubeView[0]);
	uboView.getElement(0, 0, 1).setValue(cubeView[1]);
	uboView.getElement(0, 0, 2).setValue(cubeView[2]);
	uboView.getElement(0, 0, 3).setValue(cubeView[3]);
	uboView.getElement(0, 0, 4).setValue(cubeView[4]);
	uboView.getElement(0, 0, 5).setValue(cubeView[5]);

	if (uint32_t(uboBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0) { uboBuffer->getDeviceMemory()->flushRange(); }

	const pvrvk::ClearValue clearValue = pvrvk::ClearValue(0.f, 0.f, 0.f, 0.0f);

	const pvrvk::Format format = pvrvk::Format::e_R16G16B16A16_SFLOAT;
	const pvrvk::Format outputVkFormat = pvr::utils::convertToPVRVkPixelFormat(outputFormat, pvr::ColorSpace::lRGB, outputFormatType);

	if (outputVkFormat == pvrvk::Format::e_UNDEFINED)
	{ throw InvalidArgumentError("format,type", "The provided PixelFormat and VariableType do not map to a valid Vulkan format"); }

	const uint32_t outputFormatStride = outputFormat.getBitsPerPixel() / 8;

	pvrvk::RenderPass renderpass;
	pvrvk::Framebuffer fbo;
	pvrvk::GraphicsPipeline pipeline;

	const uint32_t bufferSize = mipLevelDimensions[0] * mipLevelDimensions[0] * 6 * outputFormatStride * numMipLevels;

	// Create a buffer to use as the final destination for the image data which will be saved to disk
	pvrvk::Buffer imageDataBuffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(bufferSize, pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT | pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, allocator);

	pvrvk::WriteDescriptorSet descSetUpdate[] = {
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, descSet, 0),
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, descSet, 1),
	};

	pvrvk::Sampler sampler = device->createSampler(pvrvk::SamplerCreateInfo(pvrvk::Filter::e_LINEAR, pvrvk::Filter::e_LINEAR, pvrvk::SamplerMipmapMode::e_LINEAR,
		pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE, pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE, pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE));

	descSetUpdate[0].setImageInfo(0, pvrvk::DescriptorImageInfo(environmentMap, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
	descSetUpdate[1].setBufferInfo(0, pvrvk::DescriptorBufferInfo(uboBuffer, 0, uboView.getDynamicSliceSize()));

	device->updateDescriptorSets(descSetUpdate, ARRAY_SIZE(descSetUpdate), nullptr, 0);

	// Create an image to use as the destination for per level, per face color attachment writes
	renderTarget = pvr::utils::createImage(device,
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, format, pvrvk::Extent3D(mipLevelDimensions[0], mipLevelDimensions[0], 1),
			pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, allocator);

	// Create an image view for the render target so that it can be used a framebuffer attachment
	imageView = device->createImageView(pvrvk::ImageViewCreateInfo(renderTarget));

	// Create an image to use as the destination for per level, per face transfer operations which may involve format translation
	outputImage = pvr::utils::createImage(device,
		pvrvk::ImageCreateInfo(
			pvrvk::ImageType::e_2D, outputVkFormat, pvrvk::Extent3D(mapSize, mapSize, 1), pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT),
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, allocator);

	// create the renderpass
	pvrvk::RenderPassCreateInfo rpInfo;
	rpInfo.setAttachmentDescription(0,
		pvrvk::AttachmentDescription::createColorDescription(
			format, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_STORE));

	pvrvk::SubpassDescription subpassDesc;
	subpassDesc.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));

	rpInfo.setSubpass(0, subpassDesc);
	renderpass = device->createRenderPass(rpInfo);

	// Create the fbo
	pvrvk::FramebufferCreateInfo fboInfo;
	fboInfo.setAttachment(0, imageView); // prefilterd map
	fboInfo.setDimensions(mipLevelDimensions[0], mipLevelDimensions[0]);
	fboInfo.setRenderPass(renderpass);
	fbo = device->createFramebuffer(fboInfo);

	pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.depthStencil.enableAllStates(false);

	pipelineInfo.vertexShader = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(spv_PBRUtilsVertShader, sizeof(spv_PBRUtilsVertShader) / sizeof(spv_PBRUtilsVertShader[0])));
	pipelineInfo.fragmentShader = device->createShaderModule(
		pvrvk::ShaderModuleCreateInfo(spv_PBRUtilsPrefilteredFragShader, sizeof(spv_PBRUtilsPrefilteredFragShader) / sizeof(spv_PBRUtilsPrefilteredFragShader[0])));
	pipelineInfo.fragmentShader.setShaderConstant(0, pvrvk::ShaderConstantInfo(0, &mapNumSamples, sizeof(uint32_t)));

	// depth stencil state
	pipelineInfo.depthStencil.enableDepthWrite(false);
	pipelineInfo.depthStencil.enableDepthTest(false);

	// rasterizer state
	pipelineInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_NONE);
	pipelineInfo.rasterizer.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);
	// blend state
	pipelineInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
	// input assembler
	pipelineInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
	pipelineInfo.renderPass = renderpass;

	// vertex attributes and bindings
	pipelineInfo.vertexInput.clear();
	pipelineInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(), pvrvk::Rect2D());

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(descSet->getDescriptorSetLayout());
	pipeLayoutInfo.setPushConstantRange(0, pvrvk::PushConstantRange(pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, sizeof(float)));

	pipelineInfo.pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
	pipelineInfo.dynamicStates.setDynamicState(pvrvk::DynamicState::e_VIEWPORT, true);
	pipelineInfo.dynamicStates.setDynamicState(pvrvk::DynamicState::e_SCISSOR, true);

	pipeline = device->createGraphicsPipeline(pipelineInfo);

	float maxmip = static_cast<float>(numMipLevels - 1);

	// record commands
	cmdBuffer->begin(pvrvk::CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT);
	pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("pvr::utils::generatePreFilteredMapMipmapStyle"));

	pvr::utils::setImageLayout(outputImage, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, cmdBuffer);

	cmdBuffer->bindPipeline(pipeline);

	VkDeviceSize bufferOffset = 0;
	for (uint32_t mipLevel = 0; mipLevel < numMipLevels; ++mipLevel)
	{
		uint32_t dim = mipLevelDimensions[mipLevel];

		pvr::utils::beginCommandBufferDebugLabel(cmdBuffer, pvrvk::DebugUtilsLabel("Cubemap level"));

		// set the viewport of the current dimension
		cmdBuffer->setViewport(pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(dim), static_cast<float>(dim)));
		const pvrvk::Rect2D scissor(pvrvk::Offset2D(), pvrvk::Extent2D(dim, dim));
		cmdBuffer->setScissor(0, 1, &scissor);

		float mip = (float)mipLevel;
		float roughness = mip / maxmip;
		if (zeroRoughnessIsExternal)
		{
			// ... But, in the case where we skip the top mip level (because we plan on just using the
			// environment map for it), the equation is a bit more involved, so that
			// it correctly calculates where we switch from interpolating among lods in the prefiltered map to
			// interpolating between the environment map and the first lod of the prefiltered map
			// LOD = maxmip * (roughness - 1/maxmip) / (1 - 1/maxmip)
			// = > ... => roughness = (LOD / maxmip) * (1 - 1/maxmip) + 1/maxmip

			roughness = mip * (1.f / maxmip) * (1.f - 1.f / maxmip) + 1.f / maxmip;
		}

		// draw each face
		for (uint32_t j = 0; j < numFaces; ++j)
		{
			// set the roughness value
			cmdBuffer->pushConstants(pipeline->getPipelineLayout(), pvrvk::ShaderStageFlags::e_FRAGMENT_BIT, 0, sizeof(roughness), &roughness);

			uint32_t offset = uboView.getDynamicSliceOffset(j); // select the right orientation matrix
			cmdBuffer->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, pipeline->getPipelineLayout(), 0, descSet, &offset, 1);

			// Render to the renderTarget
			cmdBuffer->beginRenderPass(fbo, true, &clearValue, 1);
			cmdBuffer->draw(0, 6);
			cmdBuffer->endRenderPass();

			// Add a pipeline barrier for pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT -> pvrvk::AccessFlags::e_TRANSFER_READ_BIT
			{
				pvrvk::MemoryBarrierSet barriers;
				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_TRANSFER_READ_BIT, renderTarget,
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_TRANSFER_BIT, barriers);
			}

			pvrvk::Offset3D offsets[2] = { { 0, 0, 0 }, { static_cast<int32_t>(dim), static_cast<int32_t>(dim), 1 } };

			// Copy the renderTarget to outputImage performing any format translations as they are required
			const pvrvk::ImageBlit imageCopyRegions(pvrvk::ImageSubresourceLayers(), offsets, pvrvk::ImageSubresourceLayers(), offsets);
			cmdBuffer->blitImage(
				renderTarget, outputImage, &imageCopyRegions, 1, pvrvk::Filter::e_NEAREST, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL);

			// Add a pipeline barrier for the output image of pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT -> pvrvk::AccessFlags::e_TRANSFER_READ_BIT
			{
				pvrvk::MemoryBarrierSet barriers;
				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_TRANSFER_READ_BIT, outputImage,
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				cmdBuffer->pipelineBarrier(
					pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_TRANSFER_BIT | pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, barriers);
			}

			// Copy the image data to the image data buffer ready to be saved to disk
			const pvrvk::BufferImageCopy regions(bufferOffset, 0, 0, pvrvk::ImageSubresourceLayers(), pvrvk::Offset3D(), pvrvk::Extent3D(dim, dim, 1));
			cmdBuffer->copyImageToBuffer(outputImage, pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, imageDataBuffer, &regions, 1);

			// Add a pipeline barrier for the output image of pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT -> pvrvk::AccessFlags::e_HOST_READ_BIT
			{
				pvrvk::MemoryBarrierSet barriers;
				barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, pvrvk::AccessFlags::e_HOST_READ_BIT, outputImage,
					pvrvk::ImageSubresourceRange(pvrvk::ImageAspectFlags::e_COLOR_BIT), pvrvk::ImageLayout::e_TRANSFER_SRC_OPTIMAL, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL,
					queue->getFamilyIndex(), queue->getFamilyIndex()));

				cmdBuffer->pipelineBarrier(pvrvk::PipelineStageFlags::e_TRANSFER_BIT, pvrvk::PipelineStageFlags::e_HOST_BIT, barriers);
			}

			bufferOffset += outputFormatStride * dim * dim;
		}
		pvr::utils::endCommandBufferDebugLabel(cmdBuffer);
	}
	pvr::utils::endCommandBufferDebugLabel(cmdBuffer);

	cmdBuffer->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cmdBuffer;
	submitInfo.numCommandBuffers = 1;

	pvrvk::Fence fence = device->createFence();
	queue->submit(submitInfo, fence);
	fence->wait();

	// Get a pointer to the diffuse irradiance image data from the buffer and save it in to the file.
	void* data;
	data = imageDataBuffer->getDeviceMemory()->getMappedData();

	// Save the image to disk
	pvr::TextureHeader texHeader;
	texHeader.setChannelType(outputFormatType);
	texHeader.setColorSpace(pvr::ColorSpace::lRGB);
	texHeader.setDepth(1);
	texHeader.setWidth(mipLevelDimensions[0]);
	texHeader.setHeight(mipLevelDimensions[0]);
	texHeader.setNumMipMapLevels(numMipLevels);
	texHeader.setNumFaces(numFaces);
	texHeader.setNumArrayMembers(1);
	texHeader.setPixelFormat(outputFormat);

	return Texture(texHeader, static_cast<const unsigned char*>(data));
}

} // namespace utils
} // namespace pvr
  //!\endcond
