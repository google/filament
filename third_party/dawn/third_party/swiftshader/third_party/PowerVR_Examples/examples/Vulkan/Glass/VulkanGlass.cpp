/*!
\brief Demonstrates dynamic reflection and refraction by rendering two halves of the scene to a single rectangular texture.
\file  VulkanGlass.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCore/PVRCore.h"
#include "PVRShell/PVRShell.h"
#include "PVRUtils/PVRUtilsVk.h"

// vertex bindings
const pvr::utils::VertexBindings_Name VertexBindings[] = { { "POSITION", "inVertex" }, { "NORMAL", "inNormal" }, { "UV0", "inTexCoords" } };

// Shader uniforms
namespace ShaderUniforms {
enum Enum
{
	MVPMatrix,
	MVMatrix,
	MMatrix,
	InvVPMatrix,
	LightDir,
	EyePos,
	NumUniforms
};
const char* names[] = { "MVPMatrix", "MVMatrix", "MMatrix", "InvVPMatrix", "LightDir", "EyePos" };
} // namespace ShaderUniforms

enum
{
	MaxSwapChain = 4
};

// paraboloid texture size
const uint32_t ParaboloidTexSize = 1024;

// camera constants
const float CamNear = 1.0f;
const float CamFar = 5000.0f;
const float CamFov = glm::pi<float>() * 0.41f;

// textures
const char* BalloonTexFile[2] = { "BalloonTex.pvr", "BalloonTex2.pvr" };
const char CubeTexFile[] = "SkyboxTex.pvr";

// model files
const char StatueFile[] = "Satyr.pod";
const char BalloonFile[] = "Balloon.pod";

// shaders
namespace Shaders {
const char* Names[] = {
	"DefaultVertShader.vsh.spv",
	"DefaultFragShader.fsh.spv",
	"ParaboloidVertShader.vsh.spv",
	"SkyboxVertShader.vsh.spv",
	"SkyboxFragShader.fsh.spv",
	"EffectReflectVertShader.vsh.spv",
	"EffectReflectFragShader.fsh.spv",
	"EffectRefractVertShader.vsh.spv",
	"EffectRefractFragShader.fsh.spv",
	"EffectChromaticDispersion.vsh.spv",
	"EffectChromaticDispersion.fsh.spv",
	"EffectReflectionRefraction.vsh.spv",
	"EffectReflectionRefraction.fsh.spv",
	"EffectReflectChromDispersion.vsh.spv",
	"EffectReflectChromDispersion.fsh.spv",
};

enum Enum
{
	DefaultVS,
	DefaultFS,
	ParaboloidVS,
	SkyboxVS,
	SkyboxFS,
	EffectReflectVS,
	EffectReflectFS,
	EffectRefractionVS,
	EffectRefractionFS,
	EffectChromaticDispersionVS,
	EffectChromaticDispersionFS,
	EffectReflectionRefractionVS,
	EffectReflectionRefractionFS,
	EffectReflectChromDispersionVS,
	EffectReflectChromDispersionFS,
	NumShaders
};
} // namespace Shaders

// effect mappings
namespace Effects {
enum Enum
{
	ReflectChromDispersion,
	ReflectRefraction,
	Reflection,
	ChromaticDispersion,
	Refraction,
	NumEffects
};
const char* Names[Effects::NumEffects] = { "Reflection + Chromatic Dispersion", "Reflection + Refraction", "Reflection", "Chromatic Dispersion", "Refraction" };
} // namespace Effects

// clear colour for the sky
const glm::vec4 ClearSkyColor(glm::vec4(.6f, 0.8f, 1.0f, 0.0f));

struct ModelBuffers
{
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;
};

static inline pvrvk::Sampler createTrilinearImageSampler(pvrvk::Device& device)
{
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.wrapModeU = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.wrapModeV = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.wrapModeW = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	return device->createSampler(samplerInfo);
}

// an abstract base for a rendering pass - handles the drawing of different types of meshes
struct IModelPass
{
private:
	void drawMesh(pvrvk::CommandBufferBase& command, const pvr::assets::ModelHandle& modelHandle, const ModelBuffers& modelBuffers, uint32_t nodeIndex)
	{
		int32_t meshId = modelHandle->getNode(nodeIndex).getObjectId();
		const pvr::assets::Mesh& mesh = modelHandle->getMesh(meshId);

		// bind the VBO for the mesh
		command->bindVertexBuffer(modelBuffers.vbos[meshId], 0, 0);
		if (mesh.getFaces().getDataSize() != 0)
		{
			// Indexed Triangle list
			command->bindIndexBuffer(modelBuffers.ibos[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
			command->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			command->draw(0, mesh.getNumFaces() * 3, 0, 1);
		}
	}

protected:
	void drawMesh(pvrvk::SecondaryCommandBuffer& command, const pvr::assets::ModelHandle& modelHandle, const ModelBuffers& modelBuffers, uint32_t nodeIndex)
	{
		drawMesh((pvrvk::CommandBufferBase&)command, modelHandle, modelBuffers, nodeIndex);
	}

	void drawMesh(pvrvk::CommandBuffer& command, const pvr::assets::ModelHandle& modelHandle, const ModelBuffers& modelBuffers, uint32_t nodeIndex)
	{
		drawMesh((pvrvk::CommandBufferBase&)command, modelHandle, modelBuffers, nodeIndex);
	}
};

// skybox pass
struct PassSkyBox
{
	pvr::utils::StructuredBufferView _bufferMemoryView;
	pvrvk::Buffer _buffer;
	pvrvk::GraphicsPipeline _pipeline;
	pvrvk::Buffer _vbo;
	pvrvk::DescriptorSetLayout _descriptorSetLayout;
	pvr::Multi<pvrvk::DescriptorSet> _descriptorSets;
	pvrvk::ImageView _skyboxTex;
	pvrvk::Sampler _trilinearSampler;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> secondaryCommandBuffers;

	enum
	{
		UboInvViewProj,
		UboEyePos,
		UboElementCount
	};

	void update(uint32_t swapchain, const glm::mat4& invViewProj, const glm::vec3& eyePos)
	{
		_bufferMemoryView.getElement(UboInvViewProj, 0, swapchain).setValue(invViewProj);
		_bufferMemoryView.getElement(UboEyePos, 0, swapchain).setValue(glm::vec4(eyePos, 0.0f));

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{ _buffer->getDeviceMemory()->flushRange(_bufferMemoryView.getDynamicSliceOffset(swapchain), _bufferMemoryView.getDynamicSliceSize()); }
	}

	pvrvk::ImageView getSkyBox() { return _skyboxTex; }

	pvrvk::GraphicsPipeline getPipeline() { return _pipeline; }

	void initDescriptorSetLayout(pvrvk::Device& device)
	{
		// create skybox descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

		// combined image sampler descriptor
		descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		// uniform buffer
		descSetLayout.setBinding(1, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		_descriptorSetLayout = device->createDescriptorSetLayout(descSetLayout);
	}

	void initPipeline(pvr::Shell& shell, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDim, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipeInfo;

		// on screen renderpass
		pipeInfo.renderPass = renderpass;

		// load, create and set the shaders for rendering the skybox
		auto& vertexShader = Shaders::Names[Shaders::SkyboxVS];
		auto& fragmentShader = Shaders::Names[Shaders::SkyboxFS];
		std::unique_ptr<pvr::Stream> vertexShaderSource = shell.getAssetStream(vertexShader);
		std::unique_ptr<pvr::Stream> fragmentShaderSource = shell.getAssetStream(fragmentShader);

		pipeInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertexShaderSource->readToEnd<uint32_t>())));
		pipeInfo.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragmentShaderSource->readToEnd<uint32_t>())));

		// create the pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipelineLayout;
		pipelineLayout.setDescSetLayout(0, _descriptorSetLayout);

		pipeInfo.pipelineLayout = device->createPipelineLayout(pipelineLayout);

		// depth stencil state
		pipeInfo.depthStencil.enableDepthWrite(false);
		pipeInfo.depthStencil.enableDepthTest(false);

		// rasterizer state
		pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);

		// blend state
		pipeInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		// vertex attributes and bindings
		pipeInfo.vertexInput.clear();
		pipeInfo.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(float) * 3));
		pipeInfo.vertexInput.addInputAttribute(pvrvk::VertexInputAttributeDescription(0, 0, pvrvk::Format::e_R32G32B32_SFLOAT, 0));

		pipeInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDim.getWidth()), static_cast<float>(viewportDim.getHeight())),
			pvrvk::Rect2D(0, 0, viewportDim.getWidth(), viewportDim.getHeight()));

		_pipeline = device->createGraphicsPipeline(pipeInfo, pipelineCache);
	}

	void createBuffers(pvrvk::Device& device, uint32_t numSwapchain, pvr::utils::vma::Allocator& vmaAllocator)
	{
		{
			// create the sky box vbo
			static float quadVertices[] = {
				-1, 1, 0.9999f, // upper left
				-1, -1, 0.9999f, // lower left
				1, 1, 0.9999f, // upper right
				1, 1, 0.9999f, // upper right
				-1, -1, 0.9999f, // lower left
				1, -1, 0.9999f // lower right
			};

			_vbo = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(sizeof(quadVertices), pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT),
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, vmaAllocator,
				pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

			pvr::utils::updateHostVisibleBuffer(_vbo, quadVertices, 0, sizeof(quadVertices), true);
		}

		{
			// create the structured memory view
			pvr::utils::StructuredMemoryDescription desc;
			desc.addElement("InvVPMatrix", pvr::GpuDatatypes::mat4x4);
			desc.addElement("EyePos", pvr::GpuDatatypes::vec4);

			_bufferMemoryView.initDynamic(desc, numSwapchain, pvr::BufferUsageFlags::UniformBuffer,
				static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

			_buffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(_bufferMemoryView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, vmaAllocator,
				pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

			_bufferMemoryView.pointToMappedMemory(_buffer->getDeviceMemory()->getMappedData());
		}
	}

	void createDescriptorSets(pvrvk::Device& device, pvrvk::DescriptorPool& descriptorPool, pvrvk::Sampler& sampler, uint32_t numSwapchain)
	{
		pvrvk::WriteDescriptorSet writeDescSets[pvrvk::FrameworkCaps::MaxSwapChains * 2];
		// create a descriptor set per swapchain
		for (uint32_t i = 0; i < numSwapchain; ++i)
		{
			_descriptorSets.add(descriptorPool->allocateDescriptorSet(_descriptorSetLayout));
			writeDescSets[i * 2]
				.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _descriptorSets[i], 0)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_skyboxTex, sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

			writeDescSets[i * 2 + 1]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _descriptorSets[i], 1)
				.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_buffer, _bufferMemoryView.getDynamicSliceOffset(i), _bufferMemoryView.getDynamicSliceSize()));
		}
		device->updateDescriptorSets(writeDescSets, numSwapchain * 2, nullptr, 0);
	}

	void init(pvr::Shell& shell, pvrvk::Device& device, pvr::Multi<pvrvk::Framebuffer>& framebuffers, const pvrvk::RenderPass& renderpass, pvrvk::CommandBuffer setupCommandBuffer,
		pvrvk::DescriptorPool& descriptorPool, pvrvk::CommandPool& commandPool, pvrvk::PipelineCache& pipelineCache, pvr::utils::vma::Allocator& vmaBufferAllocator,
		pvr::utils::vma::Allocator& vmaImageAllocator)
	{
		_trilinearSampler = createTrilinearImageSampler(device);
		initDescriptorSetLayout(device);
		createBuffers(device, static_cast<uint32_t>(framebuffers.size()), vmaBufferAllocator);

		// load the  skybox texture
		_skyboxTex = pvr::utils::loadAndUploadImageAndView(device, CubeTexFile, true, setupCommandBuffer, shell, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
			pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, vmaBufferAllocator, vmaImageAllocator);

		createDescriptorSets(device, descriptorPool, _trilinearSampler, static_cast<uint32_t>(framebuffers.size()));
		initPipeline(shell, device, renderpass, framebuffers[0]->getDimensions(), pipelineCache);
		recordCommands(framebuffers, commandPool);
	}

	pvrvk::SecondaryCommandBuffer& getSecondaryCommandBuffer(uint32_t swapchain) { return secondaryCommandBuffers[swapchain]; }

	void recordCommands(pvr::Multi<pvrvk::Framebuffer>& framebuffers, pvrvk::CommandPool& commandPool)
	{
		for (uint32_t i = 0; i < framebuffers.size(); ++i)
		{
			secondaryCommandBuffers[i] = commandPool->allocateSecondaryCommandBuffer();
			secondaryCommandBuffers[i]->begin(framebuffers[i], 0);
			secondaryCommandBuffers[i]->bindPipeline(_pipeline);
			secondaryCommandBuffers[i]->bindVertexBuffer(_vbo, 0, 0);
			secondaryCommandBuffers[i]->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipeline->getPipelineLayout(), 0, _descriptorSets[i]);

			secondaryCommandBuffers[i]->draw(0, 6, 0, 1);

			secondaryCommandBuffers[i]->end();
		}
	}
};

// balloon pass
struct PassBalloon : public IModelPass
{
	// variable number of balloons
	enum
	{
		NumBalloon = 2
	};

	// structured memory view with entries for each balloon
	pvr::utils::StructuredBufferView _bufferMemoryView;
	pvrvk::Buffer _buffer;

	// descriptor set layout and per swap chain descriptor set
	pvrvk::DescriptorSetLayout _matrixBufferDescriptorSetLayout;
	pvr::Multi<pvrvk::DescriptorSet> _matrixDescriptorSets;

	pvrvk::DescriptorSetLayout _textureBufferDescriptorSetLayout;
	pvrvk::DescriptorSet _textureDescriptorSets[NumBalloon];

	// texture for each balloon
	pvrvk::ImageView _balloonTexures[NumBalloon];
	enum UboElement
	{
		UboElementModelViewProj,
		UboElementLightDir,
		UboElementEyePos,
		UboElementCount
	};
	enum UboBalloonIdElement
	{
		UboBalloonId
	};

	// graphics pipeline used for rendering the balloons
	pvrvk::GraphicsPipeline _pipeline;

	// container for the balloon model
	ModelBuffers _balloonBuffers;
	pvr::assets::ModelHandle _balloonScene;

	pvrvk::Sampler _trilinearSampler;

	const glm::vec3 EyePos;
	const glm::vec3 LightDir;

	pvr::Multi<pvrvk::SecondaryCommandBuffer> _secondaryCommandBuffers;

	PassBalloon() : EyePos(0.0f, 0.0f, 0.0f), LightDir(19.0f, 22.0f, -50.0f) {}

	void initDescriptorSetLayout(pvrvk::Device& device)
	{
		{
			pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;
			// uniform buffer
			descSetLayout.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
			_matrixBufferDescriptorSetLayout = device->createDescriptorSetLayout(descSetLayout);
		}

		{
			pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;
			// combined image sampler descriptor
			descSetLayout.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
			_textureBufferDescriptorSetLayout = device->createDescriptorSetLayout(descSetLayout);
		}
	}

	void createBuffers(pvrvk::Device& device, uint32_t swapchainLength, pvr::utils::vma::Allocator& vmaAllocator, pvrvk::CommandBuffer& uploadCmd)
	{
		// load the vbo and ibo data
		bool requiresCommandBufferSubmission = false;
		pvr::utils::appendSingleBuffersFromModel(device, *_balloonScene, _balloonBuffers.vbos, _balloonBuffers.ibos, uploadCmd, requiresCommandBufferSubmission, vmaAllocator);

		// create the structured memory view
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("UboElementModelViewProj", pvr::GpuDatatypes::mat4x4);
		desc.addElement("UboElementLightDir", pvr::GpuDatatypes::vec4);
		desc.addElement("UboElementEyePos", pvr::GpuDatatypes::vec4);

		_bufferMemoryView.initDynamic(desc, NumBalloon * swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_buffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(_bufferMemoryView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_bufferMemoryView.pointToMappedMemory(_buffer->getDeviceMemory()->getMappedData());
	}

	void createDescriptorSets(pvrvk::Device& device, pvrvk::Sampler& sampler, pvrvk::DescriptorPool& descpool, uint32_t numSwapchain)
	{
		pvrvk::WriteDescriptorSet writeDescSet[pvrvk::FrameworkCaps::MaxSwapChains + NumBalloon];
		uint32_t writeIndex = 0;
		// create a descriptor set per swapchain
		for (uint32_t i = 0; i < numSwapchain; ++i, ++writeIndex)
		{
			_matrixDescriptorSets.add(descpool->allocateDescriptorSet(_matrixBufferDescriptorSetLayout));

			writeDescSet[writeIndex]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _matrixDescriptorSets[i])
				.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_buffer, 0, _bufferMemoryView.getDynamicSliceSize()));
		}

		for (uint32_t i = 0; i < NumBalloon; ++i, ++writeIndex)
		{
			_textureDescriptorSets[i] = descpool->allocateDescriptorSet(_textureBufferDescriptorSetLayout);

			writeDescSet[writeIndex]
				.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _textureDescriptorSets[i])
				.setImageInfo(0, pvrvk::DescriptorImageInfo(_balloonTexures[i], sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
		}
		device->updateDescriptorSets(writeDescSet, numSwapchain + NumBalloon, nullptr, 0);
	}

	void setPipeline(pvrvk::GraphicsPipeline& pipeline) { _pipeline = pipeline; }

	void initPipeline(pvr::Shell& shell, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDim, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipeInfo;

		// on screen renderpass
		pipeInfo.renderPass = renderpass;

		// load, create and set the shaders for rendering the skybox
		auto& vertexShader = Shaders::Names[Shaders::DefaultVS];
		auto& fragmentShader = Shaders::Names[Shaders::DefaultFS];
		std::unique_ptr<pvr::Stream> vertexShaderSource = shell.getAssetStream(vertexShader);
		std::unique_ptr<pvr::Stream> fragmentShaderSource = shell.getAssetStream(fragmentShader);

		pipeInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertexShaderSource->readToEnd<uint32_t>())));
		pipeInfo.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragmentShaderSource->readToEnd<uint32_t>())));

		// create the pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipelineLayout;
		pipelineLayout.setDescSetLayout(0, _matrixBufferDescriptorSetLayout);
		pipelineLayout.setDescSetLayout(1, _textureBufferDescriptorSetLayout);

		pipeInfo.pipelineLayout = device->createPipelineLayout(pipelineLayout);

		// depth stencil state
		pipeInfo.depthStencil.enableDepthWrite(true);
		pipeInfo.depthStencil.enableDepthTest(true);

		// rasterizer state
		pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

		// blend state
		pipeInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);
		pvr::utils::populateInputAssemblyFromMesh(
			_balloonScene->getMesh(0), VertexBindings, sizeof(VertexBindings) / sizeof(VertexBindings[0]), pipeInfo.vertexInput, pipeInfo.inputAssembler);

		pipeInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDim.getWidth()), static_cast<float>(viewportDim.getHeight())),
			pvrvk::Rect2D(0, 0, viewportDim.getWidth(), viewportDim.getHeight()));

		_pipeline = device->createGraphicsPipeline(pipeInfo, pipelineCache);
	}

	void init(pvr::Shell& shell, pvrvk::Device& device, pvr::Multi<pvrvk::Framebuffer>& framebuffers, const pvrvk::RenderPass& renderpass, pvrvk::CommandBuffer& uploadCmdBuffer,
		pvrvk::DescriptorPool& descriptorPool, pvrvk::CommandPool& commandPool, const pvr::assets::ModelHandle& modelBalloon, pvrvk::PipelineCache& pipelineCache,
		pvr::utils::vma::Allocator& vmaBufferAllocator, pvr::utils::vma::Allocator& vmaImageAllocator)
	{
		_balloonScene = modelBalloon;

		_trilinearSampler = createTrilinearImageSampler(device);
		initDescriptorSetLayout(device);
		createBuffers(device, static_cast<uint32_t>(framebuffers.size()), vmaBufferAllocator, uploadCmdBuffer);

		for (uint32_t i = 0; i < NumBalloon; ++i)
		{
			_balloonTexures[i] = pvr::utils::loadAndUploadImageAndView(device, BalloonTexFile[i], true, uploadCmdBuffer, shell, pvrvk::ImageUsageFlags::e_SAMPLED_BIT,
				pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, nullptr, vmaBufferAllocator, vmaImageAllocator);
		}

		createDescriptorSets(device, _trilinearSampler, descriptorPool, static_cast<uint32_t>(framebuffers.size()));
		initPipeline(shell, device, renderpass, framebuffers[0]->getDimensions(), pipelineCache);
		recordCommands(framebuffers, commandPool);
	}

	void recordCommands(pvr::Multi<pvrvk::Framebuffer>& framebuffers, pvrvk::CommandPool& commandPool)
	{
		for (uint32_t i = 0; i < framebuffers.size(); ++i)
		{
			_secondaryCommandBuffers[i] = commandPool->allocateSecondaryCommandBuffer();
			_secondaryCommandBuffers[i]->begin(framebuffers[i], 0);

			recordCommandsIntoSecondary(_secondaryCommandBuffers[i], _bufferMemoryView, _matrixDescriptorSets[i], _bufferMemoryView.getDynamicSliceOffset(i * NumBalloon));

			_secondaryCommandBuffers[i]->end();
		}
	}

	void recordCommandsIntoSecondary(pvrvk::SecondaryCommandBuffer& command, pvr::utils::StructuredBufferView& bufferView, pvrvk::DescriptorSet& matrixDescriptorSet, uint32_t baseOffset)
	{
		command->bindPipeline(_pipeline);
		for (uint32_t i = 0; i < NumBalloon; ++i)
		{
			const uint32_t offset = bufferView.getDynamicSliceOffset(i) + baseOffset;

			command->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipeline->getPipelineLayout(), 0, matrixDescriptorSet, &offset, 1);

			command->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _pipeline->getPipelineLayout(), 1, _textureDescriptorSets[i]);
			drawMesh(command, _balloonScene, _balloonBuffers, 0);
		}
	}

	pvrvk::SecondaryCommandBuffer& getSecondaryCommandBuffer(uint32_t swapchain) { return _secondaryCommandBuffers[swapchain]; }

	void update(uint32_t swapchain, const glm::mat4 model[NumBalloon], const glm::mat4& view, const glm::mat4& proj)
	{
		for (uint32_t i = 0; i < NumBalloon; ++i)
		{
			const glm::mat4 modelView = view * model[i];
			uint32_t dynamicSlice = i + swapchain * NumBalloon;

			_bufferMemoryView.getElement(UboElementModelViewProj, 0, dynamicSlice).setValue(proj * modelView);
			// Calculate and set the model space light direction
			_bufferMemoryView.getElement(UboElementLightDir, 0, dynamicSlice).setValue(glm::normalize(glm::inverse(model[i]) * glm::vec4(LightDir, 1.0f)));
			// Calculate and set the model space eye position
			_bufferMemoryView.getElement(UboElementEyePos, 0, dynamicSlice).setValue(glm::inverse(modelView) * glm::vec4(EyePos, 0.0f));
		}

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{ _buffer->getDeviceMemory()->flushRange(_bufferMemoryView.getDynamicSliceOffset(swapchain * NumBalloon), _bufferMemoryView.getDynamicSliceSize() * NumBalloon); }
	}
};

// paraboloid pass
struct PassParabloid
{
	enum
	{
		ParabolidLeft,
		ParabolidRight,
		NumParabloid = 2
	};

private:
	enum
	{
		UboMV,
		UboLightDir,
		UboEyePos,
		UboNear,
		UboFar,
		UboCount
	};
	enum UboBalloonIdElement
	{
		UboBalloonId
	};
	const static std::pair<pvr::StringHash, pvr::GpuDatatypes> UboElementMap[UboCount];

	PassBalloon _passes[NumParabloid];
	pvrvk::GraphicsPipeline _pipelines[2];
	pvr::Multi<pvrvk::Framebuffer> _framebuffer;
	pvr::Multi<pvrvk::ImageView> _paraboloidTextures;
	pvrvk::RenderPass _renderPass;
	pvrvk::Sampler _trilinearSampler;
	pvrvk::DescriptorSetLayout _descriptorSetLayout;
	pvr::utils::StructuredBufferView _bufferMemoryView;
	pvrvk::Buffer _buffer;
	pvr::Multi<pvrvk::DescriptorSet> _matrixDescriptorSets;
	pvrvk::DescriptorSet _textureDescriptorSets[PassBalloon::NumBalloon];

	pvr::Multi<pvrvk::SecondaryCommandBuffer> _secondaryCommandBuffers;

	void initPipeline(pvr::Shell& shell, pvrvk::Device& device, const pvr::assets::ModelHandle& modelBalloon, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::Rect2D parabolidViewport[] = {
			pvrvk::Rect2D(0, 0, ParaboloidTexSize, ParaboloidTexSize), // first paraboloid (Viewport left)
			pvrvk::Rect2D(ParaboloidTexSize, 0, ParaboloidTexSize, ParaboloidTexSize) // second paraboloid (Viewport right)
		};

		// create the first pipeline for the left viewport
		pvrvk::GraphicsPipelineCreateInfo pipeInfo;

		pipeInfo.renderPass = _renderPass;

		pipeInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(Shaders::Names[Shaders::ParaboloidVS])->readToEnd<uint32_t>())));

		pipeInfo.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(Shaders::Names[Shaders::DefaultFS])->readToEnd<uint32_t>())));

		// create the pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipelineLayout;
		pipelineLayout.setDescSetLayout(0, _descriptorSetLayout);
		pipelineLayout.setDescSetLayout(1, _passes[0]._textureBufferDescriptorSetLayout);

		pipeInfo.pipelineLayout = device->createPipelineLayout(pipelineLayout);

		// blend state
		pipeInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		pvr::utils::populateInputAssemblyFromMesh(
			modelBalloon->getMesh(0), VertexBindings, sizeof(VertexBindings) / sizeof(VertexBindings[0]), pipeInfo.vertexInput, pipeInfo.inputAssembler);

		// depth stencil state
		pipeInfo.depthStencil.enableDepthWrite(true);
		pipeInfo.depthStencil.enableDepthTest(true);

		// rasterizer state
		pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_FRONT_BIT);

		// set the viewport to render to the left paraboloid
		pipeInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(static_cast<float>(parabolidViewport[0].getOffset().getX()), static_cast<float>(parabolidViewport[0].getOffset().getY()),
				static_cast<float>(parabolidViewport[0].getExtent().getWidth()), static_cast<float>(parabolidViewport[0].getExtent().getHeight())),
			pvrvk::Rect2D(0, 0, ParaboloidTexSize * 2, ParaboloidTexSize));

		// create the left paraboloid graphics pipeline
		_pipelines[0] = device->createGraphicsPipeline(pipeInfo, pipelineCache);

		// clear viewport/scissors before resetting them
		pipeInfo.viewport.clear();

		// create the second pipeline for the right viewport
		pipeInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(static_cast<float>(parabolidViewport[1].getOffset().getX()), static_cast<float>(parabolidViewport[1].getOffset().getY()),
				static_cast<float>(parabolidViewport[1].getExtent().getWidth()), static_cast<float>(parabolidViewport[1].getExtent().getHeight())),
			pvrvk::Rect2D(0, 0, ParaboloidTexSize * 2, ParaboloidTexSize));
		pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

		// create the right paraboloid graphics pipeline
		_pipelines[1] = device->createGraphicsPipeline(pipeInfo, pipelineCache);
	}

	void initFramebuffer(pvrvk::Device& device, uint32_t numSwapchain, pvr::utils::vma::Allocator& vmaAllocator)
	{
		// create the paraboloid subpass
		pvrvk::SubpassDescription subpass(pvrvk::PipelineBindPoint::e_GRAPHICS);
		// uses a single colour attachment
		subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
		// subpass uses depth stencil attachment
		subpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

		pvrvk::Format depthStencilFormat = pvrvk::Format::e_D16_UNORM;
		pvrvk::Format colorFormat = pvrvk::Format::e_R8G8B8A8_UNORM;

		// create the renderpass
		// set the final layout to ShaderReadOnlyOptimal so that the image can be bound as a texture in following passes.
		pvrvk::RenderPassCreateInfo renderPassInfo;
		// clear the image at the beginning of the renderpass and store it at the end
		// the images initial layout will be colour attachment optimal and the final layout will be shader read only optimal
		renderPassInfo.setAttachmentDescription(0,
			pvrvk::AttachmentDescription::createColorDescription(
				colorFormat, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, pvrvk::AttachmentLoadOp::e_CLEAR));

		// clear the depth stencil image at the beginning of the renderpass and ignore at the end
		renderPassInfo.setAttachmentDescription(1,
			pvrvk::AttachmentDescription::createDepthStencilDescription(depthStencilFormat, pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_DONT_CARE, pvrvk::AttachmentLoadOp::e_DONT_CARE, pvrvk::AttachmentStoreOp::e_DONT_CARE));

		renderPassInfo.setSubpass(0, subpass);

		// create the renderpass to use when rendering into the paraboloid
		_renderPass = device->createRenderPass(renderPassInfo);

		// the paraboloid will be split up into left and right sections when rendering
		const pvrvk::Extent2D framebufferDim(ParaboloidTexSize * 2, ParaboloidTexSize);
		const pvrvk::Extent3D textureDim(framebufferDim.getWidth(), framebufferDim.getHeight(), 1u);
		_framebuffer.resize(numSwapchain);

		for (uint32_t i = 0; i < numSwapchain; ++i)
		{
			//---------------
			// create the render-target colour texture and transform to
			// shader read layout so that the layout transformation
			// works properly during the command buffer recording.
			pvrvk::Image colorTexture = pvr::utils::createImage(device,
				pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, colorFormat, textureDim, pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_SAMPLED_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, vmaAllocator,
				pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

			_paraboloidTextures[i] = device->createImageView(pvrvk::ImageViewCreateInfo(colorTexture));

			//---------------
			// create the render-target depth-stencil texture
			// make depth stencil attachment transient as it is only used within this renderpass
			pvrvk::Image depthTexture = pvr::utils::createImage(device,
				pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, depthStencilFormat, textureDim,
					pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT),
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, vmaAllocator,
				pvr::utils::vma::AllocationCreateFlags::e_DEDICATED_MEMORY_BIT);

			//---------------
			// create the framebuffer
			pvrvk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.setRenderPass(_renderPass);
			framebufferInfo.setAttachment(0, _paraboloidTextures[i]);
			framebufferInfo.setAttachment(1, device->createImageView(pvrvk::ImageViewCreateInfo(depthTexture)));
			framebufferInfo.setDimensions(framebufferDim);

			_framebuffer[i] = device->createFramebuffer(framebufferInfo);
		}
	}

	void createBuffers(pvrvk::Device& device, uint32_t numSwapchain, pvr::utils::vma::Allocator& vmaAllocator)
	{
		// create the structured memory view
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement(UboElementMap[PassParabloid::UboMV].first, UboElementMap[PassParabloid::UboMV].second);
		desc.addElement(UboElementMap[PassParabloid::UboLightDir].first, UboElementMap[PassParabloid::UboLightDir].second);
		desc.addElement(UboElementMap[PassParabloid::UboEyePos].first, UboElementMap[PassParabloid::UboEyePos].second);
		desc.addElement(UboElementMap[PassParabloid::UboNear].first, UboElementMap[PassParabloid::UboNear].second);
		desc.addElement(UboElementMap[PassParabloid::UboFar].first, UboElementMap[PassParabloid::UboFar].second);

		_bufferMemoryView.initDynamic(desc, PassBalloon::NumBalloon * NumParabloid * numSwapchain, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_buffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(_bufferMemoryView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, vmaAllocator,
			pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_bufferMemoryView.pointToMappedMemory(_buffer->getDeviceMemory()->getMappedData());
	}

	void initDescriptorSetLayout(pvrvk::Device& device)
	{
		// create skybox descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

		// uniform buffer
		descSetLayout.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		_descriptorSetLayout = device->createDescriptorSetLayout(descSetLayout);
	}

	void createDescriptorSets(pvrvk::Device& device, pvrvk::DescriptorPool& descriptorPool, uint32_t numSwapchain)
	{
		pvrvk::WriteDescriptorSet descSetWrites[pvrvk::FrameworkCaps::MaxSwapChains];

		// create a descriptor set per swapchain
		for (uint32_t i = 0; i < numSwapchain; ++i)
		{
			_matrixDescriptorSets.add(descriptorPool->allocateDescriptorSet(_descriptorSetLayout));
			descSetWrites[i]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _matrixDescriptorSets[i], 0)
				.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_buffer, 0, _bufferMemoryView.getDynamicSliceSize()));
		}
		device->updateDescriptorSets(descSetWrites, numSwapchain, nullptr, 0);
	}

public:
	pvrvk::Framebuffer& getFramebuffer(uint32_t swapchainIndex) { return _framebuffer[swapchainIndex]; }

	const pvrvk::ImageView& getParaboloid(uint32_t swapchainIndex) { return _paraboloidTextures[swapchainIndex]; }

	void init(pvr::Shell& shell, pvrvk::Device& device, const pvr::assets::ModelHandle& modelBalloon, pvrvk::CommandBuffer& uploadCmdBuffer, pvrvk::CommandPool& commandPool,
		pvrvk::DescriptorPool& descriptorPool, uint32_t numSwapchain, pvrvk::PipelineCache& pipelineCache, pvr::utils::vma::Allocator& vmaBufferAllocator,
		pvr::utils::vma::Allocator& vmaImageAllocator)
	{
		initFramebuffer(device, numSwapchain, vmaImageAllocator);

		for (uint32_t i = 0; i < NumParabloid; i++)
		{
			_passes[i].init(shell, device, _framebuffer, _renderPass, uploadCmdBuffer, descriptorPool, commandPool, modelBalloon, pipelineCache, vmaBufferAllocator, vmaImageAllocator);
		}

		_trilinearSampler = createTrilinearImageSampler(device);
		initDescriptorSetLayout(device);
		createBuffers(device, numSwapchain, vmaBufferAllocator);
		createDescriptorSets(device, descriptorPool, numSwapchain);

		// create the pipeline
		initPipeline(shell, device, modelBalloon, pipelineCache);

		for (uint32_t i = 0; i < NumParabloid; i++) { _passes[i].setPipeline(_pipelines[i]); }

		recordCommands(commandPool, numSwapchain);
	}

	void update(uint32_t swapchain, const glm::mat4 balloonModelMatrices[PassBalloon::NumBalloon], const glm::vec3& position)
	{
		//--- Create the first view matrix and make it flip the X coordinate
		glm::mat4 mViewLeft = glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		mViewLeft = glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f)) * mViewLeft;

		glm::mat4 mViewRight = glm::lookAt(position, position - glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
		glm::mat4 modelView;

		// [LeftParaboloid_balloon0, LeftParaboloid_balloon1, RightParaboloid_balloon0, RightParaboloid_balloon1]
		for (uint32_t i = 0; i < PassBalloon::NumBalloon; ++i)
		{
			// left paraboloid
			{
				const uint32_t dynamicSlice = i + swapchain * PassBalloon::NumBalloon * NumParabloid;
				modelView = mViewLeft * balloonModelMatrices[i];
				_bufferMemoryView.getElement(UboMV, 0, dynamicSlice).setValue(modelView);
				_bufferMemoryView.getElement(UboLightDir, 0, dynamicSlice).setValue(glm::normalize(glm::inverse(balloonModelMatrices[i]) * glm::vec4(_passes[i].LightDir, 1.0f)));

				// Calculate and set the model space eye position
				_bufferMemoryView.getElement(UboEyePos, 0, dynamicSlice).setValue(glm::inverse(modelView) * glm::vec4(_passes[i].EyePos, 0.0f));
				_bufferMemoryView.getElement(UboNear, 0, dynamicSlice).setValue(CamNear);
				_bufferMemoryView.getElement(UboFar, 0, dynamicSlice).setValue(CamFar);

				// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
				if (static_cast<uint32_t>(_buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
				{ _buffer->getDeviceMemory()->flushRange(_bufferMemoryView.getDynamicSliceOffset(dynamicSlice), _bufferMemoryView.getDynamicSliceSize()); }
			}
			// right paraboloid
			{
				const uint32_t dynamicSlice = i + PassBalloon::NumBalloon + swapchain * PassBalloon::NumBalloon * NumParabloid;
				modelView = mViewRight * balloonModelMatrices[i];
				_bufferMemoryView.getElement(UboMV, 0, dynamicSlice).setValue(modelView);
				_bufferMemoryView.getElement(UboLightDir, 0, dynamicSlice).setValue(glm::normalize(glm::inverse(balloonModelMatrices[i]) * glm::vec4(_passes[i].LightDir, 1.0f)));

				// Calculate and set the model space eye position
				_bufferMemoryView.getElement(UboEyePos, 0, dynamicSlice).setValue(glm::inverse(modelView) * glm::vec4(_passes[i].EyePos, 0.0f));
				_bufferMemoryView.getElement(UboNear, 0, dynamicSlice).setValue(CamNear);
				_bufferMemoryView.getElement(UboFar, 0, dynamicSlice).setValue(CamFar);

				// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
				if (static_cast<uint32_t>(_buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
				{ _buffer->getDeviceMemory()->flushRange(_bufferMemoryView.getDynamicSliceOffset(dynamicSlice), _bufferMemoryView.getDynamicSliceSize()); }
			}
		}
	}

	pvrvk::SecondaryCommandBuffer& getSecondaryCommandBuffer(uint32_t swapchain) { return _secondaryCommandBuffers[swapchain]; }

	void recordCommands(pvrvk::CommandPool& commandPool, uint32_t swapchain)
	{
		for (uint32_t i = 0; i < swapchain; ++i)
		{
			_secondaryCommandBuffers[i] = commandPool->allocateSecondaryCommandBuffer();

			_secondaryCommandBuffers[i]->begin(_framebuffer[i], 0);

			// left paraboloid
			uint32_t baseOffset = _bufferMemoryView.getDynamicSliceOffset(i * PassBalloon::NumBalloon * NumParabloid);
			_passes[ParabolidLeft].recordCommandsIntoSecondary(_secondaryCommandBuffers[i], _bufferMemoryView, _matrixDescriptorSets[i], baseOffset);
			// right paraboloid

			baseOffset = _bufferMemoryView.getDynamicSliceOffset(i * PassBalloon::NumBalloon * NumParabloid + PassBalloon::NumBalloon);
			_passes[ParabolidRight].recordCommandsIntoSecondary(_secondaryCommandBuffers[i], _bufferMemoryView, _matrixDescriptorSets[i], baseOffset);

			_secondaryCommandBuffers[i]->end();
		}
	}
};

const std::pair<pvr::StringHash, pvr::GpuDatatypes> PassParabloid::UboElementMap[PassParabloid::UboCount] = {
	{ "MVMatrix", pvr::GpuDatatypes::mat4x4 },
	{ "LightDir", pvr::GpuDatatypes::vec4 },
	{ "EyePos", pvr::GpuDatatypes::vec4 },
	{ "Near", pvr::GpuDatatypes::Float },
	{ "Far", pvr::GpuDatatypes::Float },
};

struct PassStatue : public IModelPass
{
	pvrvk::GraphicsPipeline _effectPipelines[Effects::NumEffects];

	pvr::utils::StructuredBufferView _bufferMemoryView;
	pvrvk::Buffer _buffer;
	pvrvk::DescriptorSetLayout _descriptorSetLayout;
	pvr::Multi<pvrvk::DescriptorSet> _descriptorSets;
	pvrvk::Sampler _trilinearSampler;

	ModelBuffers _modelStatue;
	pvr::assets::ModelHandle _modelHandle;

	pvr::Multi<pvrvk::SecondaryCommandBuffer> _secondaryCommandBuffers;

	enum
	{
		DescSetUbo,
		DescSetParabolid,
		DescSetSkybox,
	};
	enum UboElements
	{
		MVP,
		Model,
		EyePos,
		Count
	};
	static const std::pair<pvr::StringHash, pvr::GpuDatatypes> UboElementsNames[UboElements::Count];

	void initDescriptorSetLayout(pvrvk::Device& device)
	{
		// create skybox descriptor set layout
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayout;

		// combined image sampler descriptors
		descSetLayout.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		descSetLayout.setBinding(2, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);

		// uniform buffer
		descSetLayout.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);

		_descriptorSetLayout = device->createDescriptorSetLayout(descSetLayout);
	}

	void createBuffers(pvrvk::Device& device, uint32_t numSwapchain, pvr::utils::vma::Allocator& vmaAllocator, pvrvk::CommandBuffer& uploadCmd)
	{
		// load the vbo and ibo data
		bool requiresCommandBufferSubmission = false;
		pvr::utils::appendSingleBuffersFromModel(device, *_modelHandle, _modelStatue.vbos, _modelStatue.ibos, uploadCmd, requiresCommandBufferSubmission, vmaAllocator);

		{
			// create the structured memory view
			pvr::utils::StructuredMemoryDescription desc;
			desc.addElement(UboElementsNames[UboElements::MVP].first, UboElementsNames[UboElements::MVP].second);
			desc.addElement(UboElementsNames[UboElements::Model].first, UboElementsNames[UboElements::Model].second);
			desc.addElement(UboElementsNames[UboElements::EyePos].first, UboElementsNames[UboElements::EyePos].second);

			_bufferMemoryView.initDynamic(desc, _modelHandle->getNumMeshNodes() * numSwapchain, pvr::BufferUsageFlags::UniformBuffer,
				static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
			_buffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(_bufferMemoryView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
				pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
				pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, vmaAllocator,
				pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

			_bufferMemoryView.pointToMappedMemory(_buffer->getDeviceMemory()->getMappedData());
		}
	}

	void createDescriptorSets(
		pvrvk::Device& device, PassParabloid& passParabloid, PassSkyBox& passSkybox, pvrvk::Sampler& sampler, pvrvk::DescriptorPool& descriptorPool, uint32_t numSwapchain)
	{
		pvrvk::WriteDescriptorSet writeDescSets[pvrvk::FrameworkCaps::MaxSwapChains * 3];
		// create a descriptor set per swapchain
		for (uint32_t i = 0; i < numSwapchain; ++i)
		{
			_descriptorSets.add(descriptorPool->allocateDescriptorSet(_descriptorSetLayout));
			writeDescSets[i * 3]
				.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _descriptorSets[i], 0)
				.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_buffer, 0, _bufferMemoryView.getDynamicSliceSize()));

			writeDescSets[i * 3 + 1]
				.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _descriptorSets[i], 1)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(passParabloid.getParaboloid(i), sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

			writeDescSets[i * 3 + 2]
				.set(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _descriptorSets[i], 2)
				.setImageInfo(0, pvrvk::DescriptorImageInfo(passSkybox.getSkyBox(), sampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
		}
		device->updateDescriptorSets(writeDescSets, numSwapchain * 3, nullptr, 0);
	}

	void initEffectPipelines(pvr::Shell& shell, pvrvk::Device& device, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDim, pvrvk::PipelineCache& pipelineCache)
	{
		pvrvk::GraphicsPipelineCreateInfo pipeInfo;

		// on screen renderpass
		pipeInfo.renderPass = renderpass;

		// create the pipeline layout
		pvrvk::PipelineLayoutCreateInfo pipelineLayout;
		pipelineLayout.setDescSetLayout(0, _descriptorSetLayout);

		pipeInfo.pipelineLayout = device->createPipelineLayout(pipelineLayout);

		// depth stencil state
		pipeInfo.depthStencil.enableDepthWrite(true);
		pipeInfo.depthStencil.enableDepthTest(true);

		// rasterizer state
		pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);

		// blend state
		pipeInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

		// input assembler
		pipeInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		pvr::utils::populateInputAssemblyFromMesh(_modelHandle->getMesh(0), VertexBindings, 2, pipeInfo.vertexInput, pipeInfo.inputAssembler);

		// load, create and set the shaders for rendering the skybox
		auto& vertexShader = Shaders::Names[Shaders::SkyboxVS];
		auto& fragmentShader = Shaders::Names[Shaders::SkyboxFS];
		std::unique_ptr<pvr::Stream> vertexShaderSource = shell.getAssetStream(vertexShader);
		std::unique_ptr<pvr::Stream> fragmentShaderSource = shell.getAssetStream(fragmentShader);

		pipeInfo.vertexShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertexShaderSource->readToEnd<uint32_t>())));
		pipeInfo.fragmentShader.setShader(device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragmentShaderSource->readToEnd<uint32_t>())));

		pipeInfo.viewport.setViewportAndScissor(0, pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(viewportDim.getWidth()), static_cast<float>(viewportDim.getHeight())),
			pvrvk::Rect2D(0, 0, viewportDim.getWidth(), viewportDim.getHeight()));

		pvrvk::ShaderModule shaders[Shaders::NumShaders];
		for (uint32_t i = 0; i < Shaders::NumShaders; ++i)
		{ shaders[i] = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(shell.getAssetStream(Shaders::Names[i])->readToEnd<uint32_t>())); }

		// Effects Vertex and fragment shader
		std::pair<Shaders::Enum, Shaders::Enum> effectShaders[Effects::NumEffects] = {
			{ Shaders::EffectReflectChromDispersionVS, Shaders::EffectReflectChromDispersionFS }, // ReflectChromDispersion
			{ Shaders::EffectReflectionRefractionVS, Shaders::EffectReflectionRefractionFS }, // ReflectRefraction
			{ Shaders::EffectReflectVS, Shaders::EffectReflectFS }, // Reflection
			{ Shaders::EffectChromaticDispersionVS, Shaders::EffectChromaticDispersionFS }, // ChromaticDispersion
			{ Shaders::EffectRefractionVS, Shaders::EffectRefractionFS } // Refraction
		};

		for (uint32_t i = 0; i < Effects::NumEffects; ++i)
		{
			pipeInfo.vertexShader.setShader(shaders[effectShaders[i].first]);
			pipeInfo.fragmentShader.setShader(shaders[effectShaders[i].second]);
			_effectPipelines[i] = device->createGraphicsPipeline(pipeInfo, pipelineCache);
		}
	}

	void init(pvr::Shell& shell, pvrvk::Device& device, pvrvk::CommandBuffer& uploadCmdBuffer, pvrvk::DescriptorPool& descriptorPool, uint32_t numSwapchain,
		const pvr::assets::ModelHandle& modelStatue, PassParabloid& passParabloid, PassSkyBox& passSkybox, const pvrvk::RenderPass& renderpass, const pvrvk::Extent2D& viewportDim,
		pvrvk::PipelineCache& pipelineCache, pvr::utils::vma::Allocator& vmaBufferAllocator)
	{
		_modelHandle = modelStatue;

		_trilinearSampler = createTrilinearImageSampler(device);
		initDescriptorSetLayout(device);
		createBuffers(device, numSwapchain, vmaBufferAllocator, uploadCmdBuffer);
		createDescriptorSets(device, passParabloid, passSkybox, _trilinearSampler, descriptorPool, numSwapchain);
		initEffectPipelines(shell, device, renderpass, viewportDim, pipelineCache);
	}

	void recordCommands(pvrvk::CommandPool& commandPool, uint32_t pipeEffect, pvrvk::Framebuffer& framebuffer, uint32_t swapchain)
	{
		// create the command buffer if it does not already exist
		if (!_secondaryCommandBuffers[swapchain]) { _secondaryCommandBuffers[swapchain] = commandPool->allocateSecondaryCommandBuffer(); }

		_secondaryCommandBuffers[swapchain]->begin(framebuffer, 0);

		_secondaryCommandBuffers[swapchain]->bindPipeline(_effectPipelines[pipeEffect]);
		// bind the texture and samplers and the ubos

		for (uint32_t i = 0; i < _modelHandle->getNumMeshNodes(); i++)
		{
			uint32_t offsets = _bufferMemoryView.getDynamicSliceOffset(i + swapchain * _modelHandle->getNumMeshNodes());
			_secondaryCommandBuffers[swapchain]->bindDescriptorSet(
				pvrvk::PipelineBindPoint::e_GRAPHICS, _effectPipelines[pipeEffect]->getPipelineLayout(), 0, _descriptorSets[swapchain], &offsets, 1);
			drawMesh(_secondaryCommandBuffers[swapchain], _modelHandle, _modelStatue, 0);
		}

		_secondaryCommandBuffers[swapchain]->end();
	}

	pvrvk::SecondaryCommandBuffer& getSecondaryCommandBuffer(uint32_t swapchain) { return _secondaryCommandBuffers[swapchain]; }

	void update(uint32_t swapchain, const glm::mat4& view, const glm::mat4& proj)
	{
		// The final statue transform brings him with 0.0.0 coordinates at his feet.
		// For this model we want 0.0.0 to be the around the centre of the statue, and the statue to be smaller.
		// So, we apply a transformation, AFTER all transforms that have brought him to the centre,
		// that will shrink him and move him downwards.
		static const glm::vec3 scale = glm::vec3(0.25f, 0.25f, 0.25f);
		static const glm::vec3 offset = glm::vec3(0.f, -2.f, 0.f);
		static const glm::mat4 local_transform = glm::translate(offset) * glm::scale(scale);

		for (uint32_t i = 0; i < _modelHandle->getNumMeshNodes(); ++i)
		{
			uint32_t dynamicSlice = i + swapchain * _modelHandle->getNumMeshNodes();
			const glm::mat4& modelMat = local_transform * _modelHandle->getWorldMatrix(i);
			const glm::mat3 modelMat3x3 = glm::mat3(modelMat);

			const glm::mat4& modelView = view * modelMat;
			_bufferMemoryView.getElement(UboElements::MVP, 0, dynamicSlice).setValue(proj * modelView);
			_bufferMemoryView.getElement(UboElements::Model, 0, dynamicSlice).setValue(modelMat3x3);
			_bufferMemoryView.getElement(UboElements::EyePos, 0, dynamicSlice).setValue(glm::inverse(modelView) * glm::vec4(0, 0, 0, 1));
		}

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_buffer->getDeviceMemory()->flushRange(
				_bufferMemoryView.getDynamicSliceOffset(swapchain * _modelHandle->getNumMeshNodes()), _bufferMemoryView.getDynamicSliceSize() * _modelHandle->getNumMeshNodes());
		}
	}
};

const std::pair<pvr::StringHash, pvr::GpuDatatypes> PassStatue::UboElementsNames[PassStatue::UboElements::Count]{
	{ "MVPMatrix", pvr::GpuDatatypes::mat4x4 },
	{ "MMatrix", pvr::GpuDatatypes::mat3x3 },
	{ "EyePos", pvr::GpuDatatypes::vec4 },
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvr::utils::vma::Allocator vmaAllocator;

	pvrvk::CommandPool commandPool;
	pvrvk::DescriptorPool descriptorPool;
	pvrvk::Swapchain swapchain;
	pvrvk::Queue queue;

	pvrvk::PipelineCache pipelineCache;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;

	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

	// related sets of drawing commands are grouped into "passes"
	PassSkyBox passSkyBox;
	PassParabloid passParaboloid;
	PassStatue passStatue;
	PassBalloon passBalloon;

	pvr::Multi<pvrvk::CommandBuffer> sceneCommandBuffers;
	pvr::Multi<pvrvk::SecondaryCommandBuffer> uiSecondaryCommandBuffers;
	pvr::Multi<pvrvk::ImageView> depthStencilImages;
	pvrvk::Sampler samplerTrilinear;

	pvrvk::Semaphore imageAcquiredSemaphores[uint32_t(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[uint32_t(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[uint32_t(pvrvk::FrameworkCaps::MaxSwapChains)];
	~DeviceResources()
	{
		if (device)
		{
			device->waitIdle();
			uint32_t l = swapchain->getSwapchainLength();
			for (uint32_t i = 0; i < l; ++i)
			{
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
				if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
			}
		}
	}
};

/// <summary>implementing the Shell functions.</summary>
class VulkanGlass : public pvr::Shell
{
	std::unique_ptr<DeviceResources> _deviceResources;

	// Projection, view and model matrices
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewMatrix;

	// Rotation angle for the model
	float _cameraAngle;
	float _balloonAngle[PassBalloon::NumBalloon];
	int32_t _currentEffect;
	float _tilt;
	float _currentTilt;
	uint32_t _frameId;

	pvr::assets::ModelHandle _balloonScene;
	pvr::assets::ModelHandle _statueScene;

public:
	VulkanGlass() : _tilt(0), _currentTilt(0) {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

private:
	void eventMappedInput(pvr::SimplifiedInput action);
	void updateScene(uint32_t swapchainIndex);
	void recordCommands();
};

void VulkanGlass::eventMappedInput(pvr::SimplifiedInput action)
{
	switch (action)
	{
	case pvr::SimplifiedInput::Left:
		_currentEffect -= 1;
		_currentEffect = (_currentEffect + Effects::NumEffects) % Effects::NumEffects;
		_deviceResources->uiRenderer.getDefaultDescription()->setText(Effects::Names[_currentEffect]);
		_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
		_deviceResources->device->waitIdle(); // make sure the command buffer is finished before re-recording
		recordCommands();
		break;
	case pvr::SimplifiedInput::Up: _tilt += 5.f; break;
	case pvr::SimplifiedInput::Down: _tilt -= 5.f; break;
	case pvr::SimplifiedInput::Right:
		_currentEffect += 1;
		_currentEffect = (_currentEffect + Effects::NumEffects) % Effects::NumEffects;
		_deviceResources->uiRenderer.getDefaultDescription()->setText(Effects::Names[_currentEffect]);
		_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
		_deviceResources->device->waitIdle(); // make sure the command buffer is finished before re-recording
		recordCommands();
		break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGlass::initApplication()
{
	_cameraAngle = glm::pi<float>() - .6f;

	for (uint32_t i = 0; i < PassBalloon::NumBalloon; ++i) { _balloonAngle[i] = glm::pi<float>() * i / 5.f; }

	_currentEffect = 0;
	_frameId = 0;

	// load the balloon
	_balloonScene = pvr::assets::loadModel(*this, BalloonFile);

	// load the statue
	_statueScene = pvr::assets::loadModel(*this, StatueFile);

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGlass::quitApplication()
{
	_balloonScene.reset();
	_statueScene.reset();
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGlass::initView()
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
	// create the logical device and the queues
	const pvr::utils::QueuePopulateInfo populateInfo = { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface };
	pvr::utils::QueueAccessInfo queueAccessInfo;
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &populateInfo, 1, &queueAccessInfo);
	// Get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	// Create memory allocator
	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));
	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // create the swapchain

	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	//---------------
	// Create the command pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	//---------------
	// Create the DescriptorPool
	pvrvk::DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 32)
		.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 32)
		.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 32)
		.setMaxDescriptorSets(32);

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(descPoolInfo);

	_deviceResources->sceneCommandBuffers[0] = _deviceResources->commandPool->allocateCommandBuffer();
	// Prepare the per swapchain resources
	// set Swapchain and depth-stencil attachment image initial layout
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->sceneCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->uiSecondaryCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		if (i == 0) { _deviceResources->sceneCommandBuffers[0]->begin(); }

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// set up the passes
	_deviceResources->passSkyBox.init(*this, _deviceResources->device, _deviceResources->onScreenFramebuffer, _deviceResources->onScreenFramebuffer[0]->getRenderPass(),
		_deviceResources->sceneCommandBuffers[0], _deviceResources->descriptorPool, _deviceResources->commandPool, _deviceResources->pipelineCache, _deviceResources->vmaAllocator,
		_deviceResources->vmaAllocator);

	_deviceResources->passBalloon.init(*this, _deviceResources->device, _deviceResources->onScreenFramebuffer, _deviceResources->onScreenFramebuffer[0]->getRenderPass(),
		_deviceResources->sceneCommandBuffers[0], _deviceResources->descriptorPool, _deviceResources->commandPool, _balloonScene, _deviceResources->pipelineCache,
		_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);

	_deviceResources->passParaboloid.init(*this, _deviceResources->device, _balloonScene, _deviceResources->sceneCommandBuffers[0], _deviceResources->commandPool,
		_deviceResources->descriptorPool, _deviceResources->swapchain->getSwapchainLength(), _deviceResources->pipelineCache, _deviceResources->vmaAllocator,
		_deviceResources->vmaAllocator);

	_deviceResources->passStatue.init(*this, _deviceResources->device, _deviceResources->sceneCommandBuffers[0], _deviceResources->descriptorPool,
		_deviceResources->swapchain->getSwapchainLength(), _statueScene, _deviceResources->passParaboloid, _deviceResources->passSkyBox,
		_deviceResources->onScreenFramebuffer[0]->getRenderPass(), _deviceResources->onScreenFramebuffer[0]->getDimensions(), _deviceResources->pipelineCache,
		_deviceResources->vmaAllocator);

	// Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	//---------------
	// Submit the initial commands
	_deviceResources->sceneCommandBuffers[0]->end();
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &_deviceResources->sceneCommandBuffers[0];
	submitInfo.numCommandBuffers = 1;
	_deviceResources->queue->submit(&submitInfo, 1);
	_deviceResources->queue->waitIdle(); // make sure all the uploads are finished

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Glass");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->setText(Effects::Names[_currentEffect]);
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->setText("Left / Right : Change the"
															   " effect\nUp / Down  : Tilt camera");
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();
	// Calculate the projection and view matrices
	_projectionMatrix = pvr::math::perspectiveFov(pvr::Api::Vulkan, CamFov, static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()), CamNear, CamFar,
		(isScreenRotated() ? glm::pi<float>() * .5f : 0.0f));

	recordCommands();
	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanGlass::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred</returns>
pvr::Result VulkanGlass::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	
	// make sure the commandbuffer and the semaphore are free to use.
	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	updateScene(swapchainIndex);

	//--------------------
	// Submit the graphics Commands
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags waitStage = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->sceneCommandBuffers[swapchainIndex];
	submitInfo.numCommandBuffers = 1;
	submitInfo.numWaitSemaphores = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.waitDstStageMask = &waitStage;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	//--------------------
	// Present
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numWaitSemaphores = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.imageIndices = &swapchainIndex;
	presentInfo.numSwapchains = 1;

	_deviceResources->queue->present(presentInfo);

	++_frameId;
	_frameId %= _deviceResources->swapchain->getSwapchainLength();
	return pvr::Result::Success;
}

/// <summary>Update the scene.</summary>
void VulkanGlass::updateScene(uint32_t swapchainIndex)
{
	// Fetch current time and make sure the previous time isn't greater
	uint64_t timeDifference = getFrameTime();
	// Store the current time for the next frame
	_cameraAngle += timeDifference * 0.00005f;
	for (int32_t i = 0; i < PassBalloon::NumBalloon; ++i) { _balloonAngle[i] += timeDifference * 0.0002f * (float(i) * .5f + 1.f); }

	static const glm::vec3 rotateAxis(0.0f, 1.0f, 0.0f);
	float diff = fabs(_tilt - _currentTilt);
	float diff2 = timeDifference / 20.f;
	_currentTilt += glm::sign(_tilt - _currentTilt) * (std::min)(diff, diff2);

	// Rotate the camera
	_viewMatrix = glm::lookAt(glm::vec3(0, -4, -10), glm::vec3(0, _currentTilt - 3, 0), glm::vec3(0, 1, 0)) * glm::rotate(_cameraAngle, rotateAxis);

	static glm::mat4 balloonModelMatrices[PassBalloon::NumBalloon];
	for (int32_t i = 0; i < PassBalloon::NumBalloon; ++i)
	{
		// Rotate the balloon model matrices
		balloonModelMatrices[i] = glm::rotate(_balloonAngle[i], rotateAxis) * glm::translate(glm::vec3(120.f + i * 40.f, sin(_balloonAngle[i] * 3.0f) * 20.0f, 0.0f)) *
			glm::scale(glm::vec3(3.0f, 3.0f, 3.0f));
	}
	_deviceResources->passParaboloid.update(swapchainIndex, balloonModelMatrices, glm::vec3(0, 0, 0));
	_deviceResources->passStatue.update(swapchainIndex, _viewMatrix, _projectionMatrix);
	_deviceResources->passBalloon.update(swapchainIndex, balloonModelMatrices, _viewMatrix, _projectionMatrix);
	_deviceResources->passSkyBox.update(swapchainIndex, glm::inverse(_projectionMatrix * _viewMatrix), glm::vec3(glm::inverse(_viewMatrix) * glm::vec4(0, 0, 0, 1)));
}

/// <summary>record all the secondary command buffers.</summary>
void VulkanGlass::recordCommands()
{
	pvrvk::ClearValue paraboloidPassClearValues[8];
	pvr::utils::populateClearValues(_deviceResources->passParaboloid.getFramebuffer(0)->getRenderPass(),
		pvrvk::ClearValue(ClearSkyColor.r, ClearSkyColor.g, ClearSkyColor.b, ClearSkyColor.a), pvrvk::ClearValue::createDefaultDepthStencilClearValue(), paraboloidPassClearValues);

	const pvrvk::ClearValue onScreenClearValues[2] = { pvrvk::ClearValue(ClearSkyColor.r, ClearSkyColor.g, ClearSkyColor.b, ClearSkyColor.a),
		pvrvk::ClearValue::createDefaultDepthStencilClearValue() };

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		//---------------
		// Render the UIRenderer
		_deviceResources->uiRenderer.beginRendering(_deviceResources->uiSecondaryCommandBuffers[i], _deviceResources->onScreenFramebuffer[i]);
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getDefaultDescription()->render();
		_deviceResources->uiRenderer.getDefaultControls()->render();
		_deviceResources->uiRenderer.endRendering();

		// record the statue pass with the current effect
		_deviceResources->passStatue.recordCommands(_deviceResources->commandPool, _currentEffect, _deviceResources->onScreenFramebuffer[i], i);

		_deviceResources->sceneCommandBuffers[i]->begin();

		// Render into the paraboloid
		_deviceResources->sceneCommandBuffers[i]->beginRenderPass(_deviceResources->passParaboloid.getFramebuffer(i), pvrvk::Rect2D(0, 0, 2 * ParaboloidTexSize, ParaboloidTexSize),
			false, paraboloidPassClearValues, _deviceResources->passParaboloid.getFramebuffer(i)->getNumAttachments());

		_deviceResources->sceneCommandBuffers[i]->executeCommands(_deviceResources->passParaboloid.getSecondaryCommandBuffer(i));

		_deviceResources->sceneCommandBuffers[i]->endRenderPass();

		// Create the final commandbuffer
		// make use of the paraboloid and render the other elements of the scene
		_deviceResources->sceneCommandBuffers[i]->beginRenderPass(
			_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, onScreenClearValues, ARRAY_SIZE(onScreenClearValues));

		_deviceResources->sceneCommandBuffers[i]->executeCommands(_deviceResources->passSkyBox.getSecondaryCommandBuffer(i));

		_deviceResources->sceneCommandBuffers[i]->executeCommands(_deviceResources->passBalloon.getSecondaryCommandBuffer(i));

		_deviceResources->sceneCommandBuffers[i]->executeCommands(_deviceResources->passStatue.getSecondaryCommandBuffer(i));

		_deviceResources->sceneCommandBuffers[i]->executeCommands(_deviceResources->uiSecondaryCommandBuffers[i]);

		_deviceResources->sceneCommandBuffers[i]->endRenderPass();
		_deviceResources->sceneCommandBuffers[i]->end();
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanGlass>(); }
