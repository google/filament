/*!
\brief This demo demonstrates how a particle system can be integrated efficiently to a vulkan application
\file VulkanParticleSystem.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
// Include files
// Includes the particle system class
#include "ParticleSystemGPU.h"
// enables the use of the PVRShell module which provides an abstract mechanism for the native platform primarily used for handling window creation and input handling.
#include "PVRShell/PVRShell.h"
// enables the use of the PVRUtilsVk module which provides utilities for making more efficient various Vulkan operations
// Also enables the use of PVRVk providing a reference counted object oriented wrapper for the vulkan objects
#include "PVRUtils/PVRUtilsVk.h"

namespace Files {
// Asset files
const char SphereModelFile[] = "sphere.pod";
const char FragShaderSrcFile[] = "FragShader.fsh.spv";
const char VertShaderSrcFile[] = "VertShader.vsh.spv";
const char FloorVertShaderSrcFile[] = "FloorVertShader.vsh.spv";
const char ParticleShaderFragSrcFile[] = "ParticleFragShader.fsh.spv";
const char ParticleShaderVertSrcFile[] = "ParticleVertShader.vsh.spv";
} // namespace Files

namespace Configuration {
enum
{
	MinNoParticles = 128,
	InitialNoParticles = 32768,
	MaxNoParticles = 32768 * 15,
	NumberOfSpheres = 8,
	NumDescriptorSets = 25,
	NumDynamicUniformBuffers = 25,
	NumUniformBuffers = 25,
	NumStorageBuffers = 25
};

const float CameraNear = .1f;
const float CameraFar = 1000.0f;
const glm::vec3 LightPosition(0.0f, 10.0f, 0.0f);

const Sphere Spheres[] = {
	Sphere(glm::vec3(-20.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(-20.0f, 6.0f, 0.0f), 5.f),
	Sphere(glm::vec3(-20.0f, 6.0f, 20.0f), 5.f),
	Sphere(glm::vec3(0.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(0.0f, 6.0f, 20.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, -20.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, 0.0f), 5.f),
	Sphere(glm::vec3(20.0f, 6.0f, 20.0f), 5.f),
};

const pvr::utils::StructuredMemoryDescription SpherePipeUboMapping("SpherePipelineUbo", 1,
	{
		{ "uModelViewMatrix", pvr::GpuDatatypes::mat4x4 },
		{ "uModelViewProjectionMatrix", pvr::GpuDatatypes::mat4x4 },
		{ "uModelViewITMatrix", pvr::GpuDatatypes::mat3x3 },
	});

namespace SpherePipeDynamicUboElements {
enum Enum
{
	ModelViewMatrix,
	ModelViewProjectionMatrix,
	ModelViewITMatrix,
	Count
};
}

const pvr::utils::StructuredMemoryDescription FloorPipeUboMapping("FloorPipelineUbo", 1,
	{
		{ "uModelViewMatrix", pvr::GpuDatatypes::mat4x4 },
		{ "uModelViewProjectionMatrix", pvr::GpuDatatypes::mat4x4 },
		{ "uModelViewITMatrix", pvr::GpuDatatypes::mat3x3 },
		{ "uLightPos", pvr::GpuDatatypes::vec3 },
	});

namespace FloorPipeDynamicUboElements {
enum Enum
{
	ModelViewMatrix,
	ModelViewProjectionMatrix,
	ModelViewITMatrix,
	LightPos,
	Count
};
}
} // namespace Configuration

// Index to bind the attributes to vertex shaders
namespace Attributes {
enum Enum
{
	ParticlePositionArray = 0,
	ParticleLifespanArray = 1,
	VertexArray = 0,
	NormalArray = 1,
	TexCoordArray = 2,
	BindingIndex0 = 0
};
}

struct PassSphere
{
	pvr::utils::StructuredBufferView uboPerModelBufferView;
	pvrvk::Buffer uboPerModel;
	pvr::utils::StructuredBufferView uboLightPropBufferView;
	pvrvk::Buffer uboLightProp;
	pvrvk::DescriptorSet descriptorUboPerModel;
	pvr::Multi<pvrvk::DescriptorSet> descriptorLighProp;
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::Buffer vbo;
	pvrvk::Buffer ibo;
};

struct PassParticles
{
	pvr::utils::StructuredBufferView uboMvpBufferView;
	pvrvk::Buffer uboMvp;
	pvr::Multi<pvrvk::DescriptorSet> descriptorMvp;
	pvrvk::GraphicsPipeline pipeline;
};

struct PassFloor
{
	pvr::utils::StructuredBufferView uboPerModelBufferView;
	pvrvk::Buffer uboPerModel;
	pvr::Multi<pvrvk::DescriptorSet> descriptorUbo;
	pvrvk::GraphicsPipeline pipeline;
	pvrvk::Buffer vbo;
};

/// <summary>VulkanParticleSystem is the main demo class implementing the PVRShell functions.</summary>
class VulkanParticleSystem : public pvr::Shell
{
private:
	// Resources used throughout the demo.
	struct DeviceResources
	{
		pvrvk::Instance instance;
		pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
		pvrvk::Device device;
		pvrvk::Swapchain swapchain;
		pvrvk::Queue graphicsQueue;
		pvrvk::Queue computeQueue;
		pvrvk::CommandPool commandPool;
		pvrvk::DescriptorPool descriptorPool;
		ParticleSystemGPU particleSystemGPU;

		pvr::utils::vma::Allocator vmaAllocator;

		pvr::Multi<pvrvk::CommandBuffer> graphicsCommandBuffers;
		pvr::Multi<pvrvk::SecondaryCommandBuffer> renderSpheresCommandBuffers;
		pvr::Multi<pvrvk::SecondaryCommandBuffer> renderFloorCommandBuffers;
		pvr::Multi<pvrvk::SecondaryCommandBuffer> renderParticlesCommandBuffers;
		pvr::Multi<pvrvk::SecondaryCommandBuffer> uiRendererCommandBuffers;
		pvr::Multi<pvrvk::ImageView> depthStencilImages;
		pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;

		PassSphere passSphere;
		PassParticles passParticles;
		PassFloor passFloor;
		pvrvk::DescriptorSetLayout descLayoutUboPerModel;
		pvrvk::DescriptorSetLayout descLayoutUbo;

		pvrvk::PipelineCache pipelineCache;

		pvr::Multi<pvrvk::Semaphore> imageAcquiredSemaphores;
		pvr::Multi<pvrvk::Semaphore> presentationSemaphores;
		std::vector<pvrvk::Semaphore> particleSystemSemaphores;

		pvr::Multi<pvrvk::Fence> perFrameResourcesFences;

		// UIRenderer used to display text
		pvr::ui::UIRenderer uiRenderer;

		DeviceResources(VulkanParticleSystem& thisApp) : particleSystemGPU(thisApp) {}
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

	std::unique_ptr<DeviceResources> _deviceResources;

	pvr::assets::ModelHandle _scene;
	bool _isCameraPaused;

	// View matrix
	glm::mat4 _viewMatrix, _projectionMatrix, _viewProjectionMatrix;
	glm::mat3 _viewIT;
	glm::vec3 _lightPos;
	uint32_t _frameId;
	float _angle;

public:
	VulkanParticleSystem() : _isCameraPaused(0), _angle(0.f) {}

	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();
	virtual void eventMappedInput(pvr::SimplifiedInput key);

	void createBuffers();
	void createPipelines();
	void recordMainCommandBuffer(uint32_t swapchainIndex);
	void recordUIRendererCommandBuffer(uint32_t swapchainIndex);
	void recordDrawFloorCommandBuffer(uint32_t swapchainIndex);
	void recordDrawParticlesCommandBuffer(uint32_t swapchainIndex);
	void recordDrawSpheresCommandBuffer(uint32_t idx);
	void createDescriptors();
	void updateFloor();
	void updateSpheres();
	void updateParticleSystemState();
	void updateParticleBuffers();
	void updateCamera();
};

/// <summary>Handles user input and updates live variables accordingly.</summary>
/// <param name="key">Input key to handle</param>
void VulkanParticleSystem::eventMappedInput(pvr::SimplifiedInput key)
{
	switch (key)
	{
	case pvr::SimplifiedInput::Left: {
		_deviceResources->computeQueue->waitIdle(); // wait for the queue to finish and update all the compute command buffers
		uint32_t numParticles = _deviceResources->particleSystemGPU.getNumberOfParticles();
		if (numParticles / 2 >= Configuration::MinNoParticles)
		{
			_deviceResources->particleSystemGPU.setNumberOfParticles(numParticles / 2);
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", numParticles / 2));
			_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
		}
	}
	break;
	case pvr::SimplifiedInput::Right: {
		_deviceResources->computeQueue->waitIdle(); // wait for the queue to finish and to update all the compute command buffers
		uint32_t numParticles = _deviceResources->particleSystemGPU.getNumberOfParticles();
		if (numParticles * 2 <= Configuration::MaxNoParticles)
		{
			_deviceResources->particleSystemGPU.setNumberOfParticles(numParticles * 2);
			_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", numParticles * 2));
			_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
		}
	}
	break;
	case pvr::SimplifiedInput::Action1: _isCameraPaused = !_isCameraPaused; break;
	case pvr::SimplifiedInput::ActionClose: exitShell(); break;
	default: break;
	}
}

/// <summary>Loads the mesh data required for this training course into vertex buffer objects.</summary>
void VulkanParticleSystem::createBuffers()
{
	// create the spheres vertex and index buffers
	_deviceResources->graphicsCommandBuffers[0]->begin();
	bool requiresCommandBufferSubmission = false;
	pvr::utils::createSingleBuffersFromMesh(_deviceResources->device, _scene->getMesh(0), _deviceResources->passSphere.vbo, _deviceResources->passSphere.ibo,
		_deviceResources->graphicsCommandBuffers[0], requiresCommandBufferSubmission, _deviceResources->vmaAllocator);

	_deviceResources->graphicsCommandBuffers[0]->end();

	if (requiresCommandBufferSubmission)
	{
		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &_deviceResources->graphicsCommandBuffers[0];
		submitInfo.numCommandBuffers = 1;

		// submit the queue and wait for it to become idle
		_deviceResources->graphicsQueue->submit(&submitInfo, 1);
		_deviceResources->graphicsQueue->waitIdle();
	}

	// Initialize the vertex buffer data for the floor: 3*Position data, 3* normal data
	const glm::vec2 maxCorner(40, 40);
	const float afVertexBufferData[] = { -maxCorner.x, 0.0f, -maxCorner.y, 0.0f, 1.0f, 0.0f, -maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f, maxCorner.x, 0.0f, -maxCorner.y,
		0.0f, 1.0f, 0.0f, maxCorner.x, 0.0f, maxCorner.y, 0.0f, 1.0f, 0.0f };

	_deviceResources->passFloor.vbo = pvr::utils::createBuffer(_deviceResources->device,
		pvrvk::BufferCreateInfo(sizeof(afVertexBufferData), pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
		_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);
	pvr::utils::updateHostVisibleBuffer(_deviceResources->passFloor.vbo, afVertexBufferData, 0, sizeof(afVertexBufferData), true);
}

/// <summary>Creates the shader modules and associated graphics pipelines used for rendering the scene.</summary>
void VulkanParticleSystem::createPipelines()
{
	pvrvk::ShaderModule fragShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::FragShaderSrcFile)->readToEnd<uint32_t>()));
	// Sphere Pipeline
	{
		pvrvk::ShaderModule vertShader = _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::VertShaderSrcFile)->readToEnd<uint32_t>()));
		const pvr::utils::VertexBindings attributes[] = { { "POSITION", 0 }, { "NORMAL", 1 } };
		pvrvk::GraphicsPipelineCreateInfo pipeCreateInfo;
		pipeCreateInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()),
				static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
			pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));

		pipeCreateInfo.vertexShader.setShader(vertShader);
		pipeCreateInfo.fragmentShader.setShader(fragShader);

		pipeCreateInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipeCreateInfo.depthStencil.enableDepthWrite(true).enableDepthTest(true);
		pipeCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		pipeCreateInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

		pvr::utils::populateInputAssemblyFromMesh(_scene->getMesh(0), attributes, sizeof(attributes) / sizeof(attributes[0]), pipeCreateInfo.vertexInput, pipeCreateInfo.inputAssembler);

		pipeCreateInfo.pipelineLayout = _deviceResources->device->createPipelineLayout(
			pvrvk::PipelineLayoutCreateInfo().addDescSetLayout(_deviceResources->descLayoutUboPerModel).addDescSetLayout(_deviceResources->descLayoutUbo));

		_deviceResources->passSphere.pipeline = _deviceResources->device->createGraphicsPipeline(pipeCreateInfo, _deviceResources->pipelineCache);
	}

	//  Floor Pipeline
	{
		pvrvk::ShaderModule vertShader =
			_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::FloorVertShaderSrcFile)->readToEnd<uint32_t>()));
		const pvrvk::VertexInputAttributeDescription attributes[] = { pvrvk::VertexInputAttributeDescription(0, 0, pvrvk::Format::e_R32G32B32_SFLOAT, 0),
			pvrvk::VertexInputAttributeDescription(1, 0, pvrvk::Format::e_R32G32B32_SFLOAT, sizeof(float) * 3) };
		pvrvk::GraphicsPipelineCreateInfo pipeCreateInfo;
		pipeCreateInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()),
				static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
			pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));
		pipeCreateInfo.vertexShader.setShader(vertShader);
		pipeCreateInfo.fragmentShader.setShader(fragShader);

		pipeCreateInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipeCreateInfo.depthStencil.enableDepthWrite(true).enableDepthTest(true);
		pipeCreateInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());
		pipeCreateInfo.depthStencil.enableDepthWrite(true).enableDepthTest(true);
		pipeCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		pipeCreateInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_STRIP);
		pipeCreateInfo.vertexInput.addInputAttributes(attributes, sizeof(attributes) / sizeof(attributes[0])).addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(float_t) * 6));

		pipeCreateInfo.pipelineLayout = _deviceResources->device->createPipelineLayout(pvrvk::PipelineLayoutCreateInfo().addDescSetLayout(_deviceResources->descLayoutUbo));

		pipeCreateInfo.subpass = 0;

		_deviceResources->passFloor.pipeline = _deviceResources->device->createGraphicsPipeline(pipeCreateInfo, _deviceResources->pipelineCache);
	}

	//  Particle Pipeline
	{
		const pvrvk::VertexInputAttributeDescription attributes[] = { pvrvk::VertexInputAttributeDescription(Attributes::ParticlePositionArray, 0, pvrvk::Format::e_R32G32B32_SFLOAT, 0),
			pvrvk::VertexInputAttributeDescription(Attributes::ParticleLifespanArray, 0, pvrvk::Format::e_R32_SFLOAT,
				static_cast<uint32_t>(pvr::getSize(pvr::GpuDatatypes::vec4) + pvr::getSize(pvr::GpuDatatypes::vec3))) };

		pvrvk::GraphicsPipelineCreateInfo pipeCreateInfo;

		pipeCreateInfo.viewport.setViewportAndScissor(0,
			pvrvk::Viewport(0.0f, 0.0f, static_cast<float>(_deviceResources->swapchain->getDimension().getWidth()),
				static_cast<float>(_deviceResources->swapchain->getDimension().getHeight())),
			pvrvk::Rect2D(0, 0, _deviceResources->swapchain->getDimension().getWidth(), _deviceResources->swapchain->getDimension().getHeight()));

		pipeCreateInfo.colorBlend.setAttachmentState(0,
			pvrvk::PipelineColorBlendAttachmentState(
				true, pvrvk::BlendFactor::e_SRC_ALPHA, pvrvk::BlendFactor::e_ONE, pvrvk::BlendOp::e_ADD, pvrvk::BlendFactor::e_ZERO, pvrvk::BlendFactor::e_ONE));

		pipeCreateInfo.depthStencil.enableDepthWrite(true).enableDepthTest(true);

		pipeCreateInfo.vertexShader.setShader(
			_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ParticleShaderVertSrcFile)->readToEnd<uint32_t>())));

		pipeCreateInfo.fragmentShader.setShader(
			_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(getAssetStream(Files::ParticleShaderFragSrcFile)->readToEnd<uint32_t>())));

		pipeCreateInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
		pipeCreateInfo.vertexInput.addInputAttributes(attributes, sizeof(attributes) / sizeof(attributes[0]));
		pipeCreateInfo.vertexInput.addInputBinding(pvrvk::VertexInputBindingDescription(0, sizeof(Particle)));

		pipeCreateInfo.inputAssembler.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_POINT_LIST);
		pipeCreateInfo.pipelineLayout = _deviceResources->device->createPipelineLayout(pvrvk::PipelineLayoutCreateInfo().addDescSetLayout(_deviceResources->descLayoutUbo));
		_deviceResources->passParticles.pipeline = _deviceResources->device->createGraphicsPipeline(pipeCreateInfo, _deviceResources->pipelineCache);
	}
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanParticleSystem::initApplication()
{
	// Load the _scene
	_scene = pvr::assets::loadModel(*this, Files::SphereModelFile);

	_frameId = 0;

	for (uint32_t i = 0; i < _scene->getNumMeshes(); ++i)
	{
		_scene->getMesh(i).setVertexAttributeIndex("POSITION0", Attributes::VertexArray);
		_scene->getMesh(i).setVertexAttributeIndex("NORMAL0", Attributes::NormalArray);
		_scene->getMesh(i).setVertexAttributeIndex("UV0", Attributes::TexCoordArray);
	}

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result VulkanParticleSystem::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

void VulkanParticleSystem::createDescriptors()
{
	pvrvk::DescriptorSetLayoutCreateInfo descLayoutInfo;
	// create dynamic ubo descriptor set layout
	{
		descLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		_deviceResources->descLayoutUboPerModel = _deviceResources->device->createDescriptorSetLayout(descLayoutInfo);
	}
	// create static ubo descriptor set layout
	{
		descLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT);
		_deviceResources->descLayoutUbo = _deviceResources->device->createDescriptorSetLayout(descLayoutInfo);
	}

	{
		_deviceResources->passSphere.uboPerModelBufferView.initDynamic(Configuration::SpherePipeUboMapping,
			Configuration::NumberOfSpheres * _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->passSphere.uboPerModel = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->passSphere.uboPerModelBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->passSphere.uboPerModelBufferView.pointToMappedMemory(_deviceResources->passSphere.uboPerModel->getDeviceMemory()->getMappedData());
	}

	{
		_deviceResources->passFloor.uboPerModelBufferView.initDynamic(Configuration::FloorPipeUboMapping, _deviceResources->swapchain->getSwapchainLength(),
			pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->passFloor.uboPerModel = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->passFloor.uboPerModelBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->passFloor.uboPerModelBufferView.pointToMappedMemory(_deviceResources->passFloor.uboPerModel->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("uLightPosition", pvr::GpuDatatypes::vec3);

		_deviceResources->passSphere.uboLightPropBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->passSphere.uboLightProp = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->passSphere.uboLightPropBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->passSphere.uboLightPropBufferView.pointToMappedMemory(_deviceResources->passSphere.uboLightProp->getDeviceMemory()->getMappedData());
	}

	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("uModelViewProjectionMatrix", pvr::GpuDatatypes::mat4x4);

		_deviceResources->passParticles.uboMvpBufferView.initDynamic(desc, _deviceResources->swapchain->getSwapchainLength(), pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->passParticles.uboMvp = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->passParticles.uboMvpBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->passParticles.uboMvpBufferView.pointToMappedMemory(_deviceResources->passParticles.uboMvp->getDeviceMemory()->getMappedData());
	}

	std::vector<pvrvk::WriteDescriptorSet> descSetWrites;
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();

	// create the ubo dynamic descriptor set
	_deviceResources->passSphere.descriptorUboPerModel = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descLayoutUboPerModel);

	descSetWrites.push_back(
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->passSphere.descriptorUboPerModel, 0)
			.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->passSphere.uboPerModel, 0, _deviceResources->passSphere.uboPerModelBufferView.getDynamicSliceSize())));

	for (uint32_t i = 0; i < swapchainLength; ++i)
	{
		// sphere descriptors
		{
			// create the ubo static descriptor set
			_deviceResources->passSphere.descriptorLighProp[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descLayoutUbo);

			descSetWrites.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->passSphere.descriptorLighProp[i], 0)
					.setBufferInfo(0,
						pvrvk::DescriptorBufferInfo(_deviceResources->passSphere.uboLightProp, _deviceResources->passSphere.uboLightPropBufferView.getDynamicSliceOffset(i),
							_deviceResources->passSphere.uboLightPropBufferView.getDynamicSliceSize())));
		}

		// particle descriptor
		{
			_deviceResources->passParticles.descriptorMvp[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descLayoutUbo);

			descSetWrites.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->passParticles.descriptorMvp[i], 0)
					.setBufferInfo(0,
						pvrvk::DescriptorBufferInfo(_deviceResources->passParticles.uboMvp, _deviceResources->passParticles.uboMvpBufferView.getDynamicSliceOffset(i),
							_deviceResources->passParticles.uboMvpBufferView.getDynamicSliceSize())));
		}

		// floor descriptors
		{
			// create the ubo dynamic descriptor set
			_deviceResources->passFloor.descriptorUbo[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->descLayoutUbo);
			descSetWrites.push_back(
				pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->passFloor.descriptorUbo[i], 0)
					.setBufferInfo(0,
						pvrvk::DescriptorBufferInfo(_deviceResources->passFloor.uboPerModel, _deviceResources->passFloor.uboPerModelBufferView.getDynamicSliceOffset(i),
							_deviceResources->passFloor.uboPerModelBufferView.getDynamicSliceSize())));
		}
	}
	_deviceResources->device->updateDescriptorSets(descSetWrites.data(), static_cast<uint32_t>(descSetWrites.size()), nullptr, 0);
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanParticleSystem::initView()
{
	_deviceResources = std::make_unique<DeviceResources>(*this);

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

	// Request retrieval of 2 queues:
	// 1. A queue which supports graphics commands and which can also be used to present to the specified surface
	// 2. A queue which supports compute commands. This queue may be the same queue as (1.), may be another queue in the same queue family or may be from another
	//	queue family entirely.
	pvr::utils::QueuePopulateInfo queueCreateInfos[] = { { pvrvk::QueueFlags::e_GRAPHICS_BIT, surface }, { pvrvk::QueueFlags::e_COMPUTE_BIT } };

	pvr::utils::QueueAccessInfo queueAccessInfos[2];
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), queueCreateInfos, 2, queueAccessInfos);

	// There is no need to check for validity of the compute queue as pvr::utils::createDeviceAndQueues in the worst case will return the same queue for 1. and 2.
	_deviceResources->graphicsQueue = _deviceResources->device->getQueue(queueAccessInfos[0].familyId, queueAccessInfos[0].queueId);
	_deviceResources->computeQueue = _deviceResources->device->getQueue(queueAccessInfos[1].familyId, queueAccessInfos[1].queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Create the command pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->graphicsQueue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	// Create the descriptor pool
	pvrvk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, Configuration::NumDynamicUniformBuffers)
		.addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, Configuration::NumUniformBuffers)
		.addDescriptorInfo(pvrvk::DescriptorType::e_STORAGE_BUFFER, Configuration::NumStorageBuffers)
		.setMaxDescriptorSets(Configuration::NumDescriptorSets);
	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(poolInfo);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; }

	// Create the Swapchain, its renderpass, attachments and framebuffers. Will support MSAA if enabled through command line.
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// Create the per swapchain command buffers, semaphores and fences.
	_deviceResources->particleSystemSemaphores.reserve(_deviceResources->swapchain->getSwapchainLength());
	for (uint8_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->graphicsCommandBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->renderSpheresCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->renderFloorCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->renderParticlesCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();
		_deviceResources->uiRendererCommandBuffers[i] = _deviceResources->commandPool->allocateSecondaryCommandBuffer();

		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->particleSystemSemaphores.emplace_back(_deviceResources->device->createSemaphore());
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}

	// Initialize UIRenderer textures
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->graphicsQueue);

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// Create a set of spheres to use in the particle system
	const std::vector<Sphere> spheres(Configuration::Spheres, Configuration::Spheres + Configuration::NumberOfSpheres);

	// Initialise the particle system providing an array of semaphores on which the example will wait for particle system simulation completion
	_deviceResources->particleSystemGPU.init(Configuration::MaxNoParticles, spheres, _deviceResources->device, _deviceResources->computeQueue, _deviceResources->descriptorPool,
		_deviceResources->vmaAllocator, _deviceResources->pipelineCache, _deviceResources->particleSystemSemaphores);

	// Create the buffers
	createBuffers();

	// Create the descriptor sets used for rendering the scene and particles
	createDescriptors();

	// Create the graphics pipeline used for rendering the scene and particles
	createPipelines();

	// Creates the projection matrix.
	_projectionMatrix = pvr::math::perspectiveFov(
		pvr::Api::Vulkan, glm::pi<float>() / 3.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight()), Configuration::CameraNear, Configuration::CameraFar);

	// Initialise particle system properties. These properties will affect the next call to step
	_deviceResources->particleSystemGPU.setGravity(glm::vec3(0.f, -9.81f, 0.f));
	_deviceResources->particleSystemGPU.setNumberOfParticles(Configuration::InitialNoParticles);

	// Initialise UI Renderer text
	_deviceResources->uiRenderer.getDefaultTitle()->setText("ParticleSystem");
	_deviceResources->uiRenderer.getDefaultDescription()->setText(pvr::strings::createFormatted("No. of Particles: %d", _deviceResources->particleSystemGPU.getNumberOfParticles()));
	_deviceResources->uiRenderer.getDefaultControls()->setText("Action1: Pause rotation\nLeft: Decrease particles\n"
															   "Right: Increase particles");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultDescription()->commitUpdates();
	_deviceResources->uiRenderer.getDefaultControls()->commitUpdates();

	// Record commands rendering the UI, drawing the floor and the spheres
	for (uint8_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		recordUIRendererCommandBuffer(i);
		recordDrawFloorCommandBuffer(i);
		recordDrawSpheresCommandBuffer(i);
	}

	updateCamera();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanParticleSystem::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanParticleSystem::renderFrame()
{
	// Update the particle system states prior to calling step
	updateParticleSystemState();

	// Advance the particle system one tick
	// The particle system is updated prior to calling acquire next image and is not directly coupled to the image presentation logic
	// The semaphore returned from the particle system step must be waited on prior to making use of the particle system resources for the current frame
	// The frame id provided to the particle system step call is used in the particle system to prevent updating resources in use by the specified frame
	pvrvk::Semaphore waitParticleSystemSemaphore = _deviceResources->particleSystemGPU.step(_frameId);

	// Acquire an image for the current frame
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	// Wait for the resources for the current swapchain index prior to making use of them
	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	updateCamera();

	// Update scene resources
	updateParticleBuffers();
	updateFloor();
	updateSpheres();

	// Re-record the command buffer used to render the particles for the current frame
	// The vertex buffer bound and used to provide particle positions is retrieved from the particle system and must be synced up with the rendering commands
	// for the current frame.
	recordDrawParticlesCommandBuffer(swapchainIndex);

	// Sync up the newly recorded secondary command buffers with the statically recorded command buffers into a Main command buffer to be submitted
	recordMainCommandBuffer(swapchainIndex);

	{
		pvrvk::SubmitInfo submitInfo;

		// PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT may only take place once the image acquisition semaphore has been signalled (signalled via acquireNextImage)
		// PipelineStageFlags::e_VERTEX_INPUT_BIT may only take place once the particle system semaphore has been signalled by the queue submission made in the
		//	particle system step call.
		pvrvk::PipelineStageFlags pipeWaitStageFlags[] = { pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_VERTEX_INPUT_BIT };
		submitInfo.commandBuffers = &_deviceResources->graphicsCommandBuffers[swapchainIndex];
		submitInfo.numCommandBuffers = 1;
		pvrvk::Semaphore graphicsWaitSemaphores[] = { _deviceResources->imageAcquiredSemaphores[_frameId], waitParticleSystemSemaphore };
		submitInfo.waitSemaphores = graphicsWaitSemaphores;
		submitInfo.numWaitSemaphores = 2;

		// The completion of commands will cause signalling of the presentation semaphore and the particle system semaphore for the current frame
		// The presentation semaphore guarantees that only completed images are presented to the screen
		// The particle system semaphore guarantees that subsequent particle system updates may take place without trampling on in-use resources
		pvrvk::Semaphore graphicsSignalSemaphores[] = { _deviceResources->presentationSemaphores[_frameId], _deviceResources->particleSystemSemaphores[_frameId] };
		submitInfo.signalSemaphores = graphicsSignalSemaphores;
		submitInfo.numSignalSemaphores = 2;
		submitInfo.waitDstStageMask = pipeWaitStageFlags;
		_deviceResources->graphicsQueue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);
	}

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->graphicsQueue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, this->getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// Handle presentation of the current image to the screen
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.numSwapchains = 1;
	presentInfo.imageIndices = &swapchainIndex;
	_deviceResources->graphicsQueue->present(presentInfo);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/// <summary>Updates the memory from where the command buffer will read the values to render the spheres.</summary>
void VulkanParticleSystem::updateSpheres()
{
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	pvr::utils::StructuredBufferView& bufferView = _deviceResources->passSphere.uboPerModelBufferView;

	{
		glm::mat4 modelView;
		for (uint32_t i = 0; i < Configuration::NumberOfSpheres; ++i)
		{
			uint32_t dynamicSlice = i + swapchainIndex * Configuration::NumberOfSpheres;

			const glm::vec3& position = Configuration::Spheres[i].vPosition;
			float radius = Configuration::Spheres[i].fRadius;
			modelView = _viewMatrix * glm::translate(position) * glm::scale(glm::vec3(radius));
			bufferView.getElement(Configuration::SpherePipeDynamicUboElements::ModelViewMatrix, 0, dynamicSlice).setValue(modelView);
			bufferView.getElement(Configuration::SpherePipeDynamicUboElements::ModelViewProjectionMatrix, 0, dynamicSlice).setValue(_projectionMatrix * modelView);
			bufferView.getElement(Configuration::SpherePipeDynamicUboElements::ModelViewITMatrix, 0, dynamicSlice).setValue(glm::mat3x4(glm::inverseTranspose(glm::mat3(modelView))));
		}

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(_deviceResources->passSphere.uboPerModel->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			_deviceResources->passSphere.uboPerModel->getDeviceMemory()->flushRange(
				bufferView.getDynamicSliceOffset(swapchainIndex * Configuration::NumberOfSpheres), bufferView.getDynamicSliceSize() * Configuration::NumberOfSpheres);
		}
	}

	_deviceResources->passSphere.uboLightPropBufferView.getElement(0, 0, swapchainIndex).setValue(_lightPos);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->passSphere.uboLightProp->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->passSphere.uboLightProp->getDeviceMemory()->flushRange(
			_deviceResources->passSphere.uboLightPropBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->passSphere.uboLightPropBufferView.getDynamicSliceSize());
	}
}

/// <summary>Updates the memory from where the command buffer will read the values to render the floor.</summary>
void VulkanParticleSystem::updateFloor()
{
	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();
	pvr::utils::StructuredBufferView& uboView = _deviceResources->passFloor.uboPerModelBufferView;

	uboView.getElement(Configuration::FloorPipeDynamicUboElements::ModelViewMatrix, 0, swapchainIndex).setValue(_viewMatrix);
	uboView.getElement(Configuration::FloorPipeDynamicUboElements::ModelViewProjectionMatrix, 0, swapchainIndex).setValue(_viewProjectionMatrix);
	uboView.getElement(Configuration::FloorPipeDynamicUboElements::ModelViewITMatrix, 0, swapchainIndex).setValue(_viewIT);
	uboView.getElement(Configuration::FloorPipeDynamicUboElements::LightPos, 0, swapchainIndex).setValue(_lightPos);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->passFloor.uboPerModel->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ _deviceResources->passFloor.uboPerModel->getDeviceMemory()->flushRange(uboView.getDynamicSliceOffset(swapchainIndex), uboView.getDynamicSliceSize()); }
}

/// <summary>Updates the particle system state affecting subsequent step commands.</summary>
void VulkanParticleSystem::updateParticleSystemState()
{
	float dt = static_cast<float>(getFrameTime());

	static float rot_angle = 0.0f;
	rot_angle += dt / 500.0f;
	float el_angle = (sinf(rot_angle / 4.0f) + 1.0f) * 0.2f + 0.2f;

	glm::mat4 rot = glm::rotate(rot_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 skew = glm::rotate(el_angle, glm::vec3(0.0f, 0.0f, 1.0f));

	Emitter sEmitter(rot * skew, 1.3f, 1.0f);

	_deviceResources->particleSystemGPU.setEmitter(sEmitter);
	_deviceResources->particleSystemGPU.updateTime(dt);
}

/// <summary>Updates the particle buffers.</summary>
void VulkanParticleSystem::updateParticleBuffers()
{
	uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->passParticles.uboMvpBufferView.getElement(0, 0, swapchainIndex).setValue(_viewProjectionMatrix);

	// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
	if (static_cast<uint32_t>(_deviceResources->passParticles.uboMvp->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{
		_deviceResources->passParticles.uboMvp->getDeviceMemory()->flushRange(
			_deviceResources->passParticles.uboMvpBufferView.getDynamicSliceOffset(swapchainIndex), _deviceResources->passParticles.uboMvpBufferView.getDynamicSliceSize());
	}
}

/// <summary>Record the main command buffer for the given frame.</summary>
/// <param name="swapchainIndex">The swapchain index signifying the frame to record commands to render.</param>
void VulkanParticleSystem::recordMainCommandBuffer(uint32_t swapchainIndex)
{
	_deviceResources->graphicsCommandBuffers[swapchainIndex]->reset();
	const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.0f, 0.0f, 0.0f, 1.0f), pvrvk::ClearValue::createDefaultDepthStencilClearValue() };
	_deviceResources->graphicsCommandBuffers[swapchainIndex]->begin();
	_deviceResources->graphicsCommandBuffers[swapchainIndex]->beginRenderPass(
		_deviceResources->onScreenFramebuffer[swapchainIndex], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), false, clearValues, ARRAY_SIZE(clearValues));

	_deviceResources->graphicsCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->renderFloorCommandBuffers[swapchainIndex]);
	_deviceResources->graphicsCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->renderSpheresCommandBuffers[swapchainIndex]);
	_deviceResources->graphicsCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->renderParticlesCommandBuffers[swapchainIndex]);
	_deviceResources->graphicsCommandBuffers[swapchainIndex]->executeCommands(_deviceResources->uiRendererCommandBuffers[swapchainIndex]);

	_deviceResources->graphicsCommandBuffers[swapchainIndex]->endRenderPass();
	_deviceResources->graphicsCommandBuffers[swapchainIndex]->end();
}

/// <summary>Record the commands used for rendering the UI.</summary>
/// <param name="swapchainIndex">The swapchain index signifying the frame to record commands to render.</param>
void VulkanParticleSystem::recordUIRendererCommandBuffer(uint32_t swapchainIndex)
{
	_deviceResources->uiRendererCommandBuffers[swapchainIndex]->begin(
		_deviceResources->onScreenFramebuffer[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
	_deviceResources->uiRenderer.beginRendering(_deviceResources->uiRendererCommandBuffers[swapchainIndex], _deviceResources->onScreenFramebuffer[swapchainIndex], true);
	_deviceResources->uiRenderer.getDefaultTitle()->render();
	_deviceResources->uiRenderer.getDefaultDescription()->render();
	_deviceResources->uiRenderer.getDefaultControls()->render();
	_deviceResources->uiRenderer.getSdkLogo()->render();
	_deviceResources->uiRenderer.endRendering();
	_deviceResources->uiRendererCommandBuffers[swapchainIndex]->end();
}

/// <summary>Record the commands used for rendering the particles.</summary>
/// <param name="swapchainIndex">The swapchain index signifying the frame to record commands to render.</param>
void VulkanParticleSystem::recordDrawParticlesCommandBuffer(uint32_t swapchainIndex)
{
	_deviceResources->renderParticlesCommandBuffers[swapchainIndex]->reset();
	_deviceResources->renderParticlesCommandBuffers[swapchainIndex]->begin(
		_deviceResources->onScreenFramebuffer[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	_deviceResources->renderParticlesCommandBuffers[swapchainIndex]->bindPipeline(_deviceResources->passParticles.pipeline);
	_deviceResources->renderParticlesCommandBuffers[swapchainIndex]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->passParticles.pipeline->getPipelineLayout(), 0, _deviceResources->passParticles.descriptorMvp[swapchainIndex]);

	const pvrvk::Buffer& outputParticleSystemBuffer = _deviceResources->particleSystemGPU.getParticleSystemBuffer();
	_deviceResources->renderParticlesCommandBuffers[swapchainIndex]->bindVertexBuffer(outputParticleSystemBuffer, 0, 0);
	_deviceResources->renderParticlesCommandBuffers[swapchainIndex]->draw(0, _deviceResources->particleSystemGPU.getNumberOfParticles(), 0, 1);

	_deviceResources->renderParticlesCommandBuffers[swapchainIndex]->end();
}

/// <summary>Record the commands used for rendering the spheres.</summary>
/// <param name="swapchainIndex">The swapchain index signifying the frame to record commands to render.</param>
void VulkanParticleSystem::recordDrawSpheresCommandBuffer(uint32_t swapchainIndex)
{
	_deviceResources->renderSpheresCommandBuffers[swapchainIndex]->begin(
		_deviceResources->onScreenFramebuffer[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	_deviceResources->renderSpheresCommandBuffers[swapchainIndex]->bindPipeline(_deviceResources->passSphere.pipeline);
	_deviceResources->renderSpheresCommandBuffers[swapchainIndex]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->passSphere.pipeline->getPipelineLayout(), 1, _deviceResources->passSphere.descriptorLighProp[0]);

	for (uint32_t i = 0; i < Configuration::NumberOfSpheres; i++)
	{
		static const pvr::assets::Mesh& mesh = _scene->getMesh(0);
		uint32_t offset = _deviceResources->passSphere.uboPerModelBufferView.getDynamicSliceOffset(i + swapchainIndex * Configuration::NumberOfSpheres);
		_deviceResources->renderSpheresCommandBuffers[swapchainIndex]->bindDescriptorSet(
			pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->passSphere.pipeline->getPipelineLayout(), 0, _deviceResources->passSphere.descriptorUboPerModel, &offset, 1);

		_deviceResources->renderSpheresCommandBuffers[swapchainIndex]->bindVertexBuffer(_deviceResources->passSphere.vbo, 0, 0);
		_deviceResources->renderSpheresCommandBuffers[swapchainIndex]->bindIndexBuffer(_deviceResources->passSphere.ibo, 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
		// Indexed Triangle list
		_deviceResources->renderSpheresCommandBuffers[swapchainIndex]->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
	}

	_deviceResources->renderSpheresCommandBuffers[swapchainIndex]->end();
}

/// <summary>Record the commands used for rendering the floor.</summary>
/// <param name="swapchainIndex">The swapchain index signifying the frame to record commands to render.</param>
void VulkanParticleSystem::recordDrawFloorCommandBuffer(uint32_t swapchainIndex)
{
	_deviceResources->renderFloorCommandBuffers[swapchainIndex]->begin(
		_deviceResources->onScreenFramebuffer[swapchainIndex], 0, pvrvk::CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	// Enables depth testing
	// We need to calculate the texture projection matrix. This matrix takes the pixels from world space to previously rendered light projection space
	// where we can look up values from our saved depth buffer. The matrix is constructed from the light view and projection matrices as used for the previous render and
	// then multiplied by the inverse of the current view matrix.
	_deviceResources->renderFloorCommandBuffers[swapchainIndex]->bindPipeline(_deviceResources->passFloor.pipeline);
	_deviceResources->renderFloorCommandBuffers[swapchainIndex]->bindDescriptorSet(
		pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->passFloor.pipeline->getPipelineLayout(), 0, _deviceResources->passFloor.descriptorUbo[swapchainIndex]);
	_deviceResources->renderFloorCommandBuffers[swapchainIndex]->bindVertexBuffer(_deviceResources->passFloor.vbo, 0, 0);
	// Draw the quad
	_deviceResources->renderFloorCommandBuffers[swapchainIndex]->draw(0, 4);

	_deviceResources->renderFloorCommandBuffers[swapchainIndex]->end();
}

/// <summary>Updates the camera state.</summary>
void VulkanParticleSystem::updateCamera()
{
	if (!_isCameraPaused) { _angle += getFrameTime() / 5000.0f; }
	{
		_angle += (getFrameTime() / 500.f) * (isKeyPressed(pvr::Keys::D) - isKeyPressed(pvr::Keys::A));
	}
	glm::vec3 vFrom = glm::vec3(sinf(_angle) * 50.0f, 30.0f, cosf(_angle) * 50.0f);

	_viewMatrix = glm::lookAt(vFrom, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	_viewIT = glm::inverseTranspose(glm::mat3(_viewMatrix));
	_lightPos = glm::vec3(_viewMatrix * glm::vec4(Configuration::LightPosition, 1.0f));
	_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanParticleSystem>(); }
