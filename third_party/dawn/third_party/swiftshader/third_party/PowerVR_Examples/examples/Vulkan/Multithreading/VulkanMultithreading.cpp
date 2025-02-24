/*!
\brief Shows how implement multithreading in a vulkan project.
\file VulkanMultithreading.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/PVRShell.h"
#include "PVRUtils/Vulkan/AsynchronousVk.h"
#include "PVRUtils/PVRUtilsVk.h"
#include <mutex>

const float RotateY = glm::pi<float>() / 150;
const glm::vec4 LightDir(.24f, .685f, -.685f, 0.0f);
const pvrvk::ClearValue ClearValue(0.0f, 0.40f, .39f, 1.f);

// vertex attributes
namespace VertexAttrib {
enum Enum
{
	VertexArray,
	NormalArray,
	TexCoordArray,
	TangentArray,
	numAttribs
};
}

const pvr::utils::VertexBindings VertexAttribBindings[] = {
	{ "POSITION", 0 },
	{ "NORMAL", 1 },
	{ "UV0", 2 },
	{ "TANGENT", 3 },
};

// shader uniforms
namespace Uniform {
enum Enum
{
	MVPMatrix,
	LightDir,
	NumUniforms
};
}

// Content file names

// Source and binary shaders
const char FragShaderSrcFile[] = "FragShader.fsh.spv";
const char VertShaderSrcFile[] = "VertShader.vsh.spv";

// POD _scene files
const char SceneFile[] = "Satyr.pod";

struct UboPerMeshData
{
	glm::mat4 mvpMtx;
	glm::vec3 lightDirModel;
};

struct DescriptorSetUpdateRequiredInfo
{
	pvr::utils::AsyncApiTexture diffuseTex;
	pvr::utils::AsyncApiTexture bumpTex;
	pvrvk::Sampler trilinearSampler;
	pvrvk::Sampler bilinearSampler;
};

struct DeviceResources
{
	pvrvk::Instance instance;
	pvr::utils::DebugUtilsCallbacks debugUtilsCallbacks;
	pvrvk::Device device;
	pvrvk::Swapchain swapchain;
	pvrvk::Queue queue;

	pvr::utils::vma::Allocator vmaAllocator;

	pvrvk::DescriptorPool descriptorPool;
	pvrvk::CommandPool commandPool;

	pvr::Multi<pvrvk::CommandBuffer> cmdBuffers; // per swapchain
	pvr::Multi<pvrvk::CommandBuffer> loadingTextCmdBuffer; // per swapchain

	pvr::Multi<pvrvk::Framebuffer> onScreenFramebuffer;
	pvr::Multi<pvrvk::ImageView> depthStencilImages;

	pvrvk::Semaphore imageAcquiredSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	pvrvk::GraphicsPipeline pipe;

	pvr::async::TextureAsyncLoader loader;
	pvr::utils::ImageApiAsyncUploader uploader;
	std::vector<pvrvk::Buffer> vbos;
	std::vector<pvrvk::Buffer> ibos;
	pvrvk::DescriptorSetLayout texLayout;
	pvrvk::DescriptorSetLayout uboLayoutDynamic;
	pvrvk::PipelineLayout pipelayout;
	pvrvk::DescriptorSet texDescSet;

	// UIRenderer used to display text
	pvr::ui::UIRenderer uiRenderer;
	pvr::ui::Text loadingText[3];
	pvr::utils::StructuredBufferView structuredMemoryView;
	pvrvk::Buffer ubo;
	pvrvk::DescriptorSet uboDescSet[4];

	pvrvk::PipelineCache pipelineCache;

	DescriptorSetUpdateRequiredInfo asyncUpdateInfo;

	~DeviceResources()
	{
		if (device) { device->waitIdle(); }
		auto items_remaining = loader.getNumQueuedItems();
		if (items_remaining)
		{
			Log(LogLevel::Information,
				"Asynchronous Texture Loader is not done: %d items pending. Before releasing,"
				" will wait until all pending load jobs are done.",
				items_remaining);
		}
		items_remaining = uploader.getNumQueuedItems();
		if (items_remaining)
		{
			Log(LogLevel::Information,
				"Asynchronous Texture Uploader is not done: %d items pending. Before releasing,"
				" will wait until all pending load jobs are done.",
				items_remaining);
		}
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

/// <summary>Class implementing the Shell functions.</summary>
class VulkanMultithreading : public pvr::Shell
{
	pvr::async::Mutex _hostMutex;

	// 3D Model
	pvr::assets::ModelHandle _scene;

	// Projection and view matrix
	glm::mat4 _viewProj;

	bool _loadingDone;
	// The translation and Rotate parameter of Model
	float _angleY;
	uint32_t _frameId;
	std::unique_ptr<DeviceResources> _deviceResources;

public:
	VulkanMultithreading() : _loadingDone(false) {}
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

	void createImageSamplerDescriptorSets();
	void createUbo();
	void loadPipeline();
	void drawMesh(pvrvk::CommandBuffer& cmdBuffers, int i32NodeIndex);
	void recordMainCommandBuffer();
	void recordLoadingCommandBuffer();
	void updateTextureDescriptorSet();
};

void VulkanMultithreading::updateTextureDescriptorSet()
{
	// create the descriptor set
	pvrvk::WriteDescriptorSet writeDescInfo[2] = { pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->texDescSet, 0),
		pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->texDescSet, 1) };

	writeDescInfo[0].setImageInfo(0,
		pvrvk::DescriptorImageInfo(
			_deviceResources->asyncUpdateInfo.diffuseTex->get(), _deviceResources->asyncUpdateInfo.bilinearSampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	writeDescInfo[1].setImageInfo(0,
		pvrvk::DescriptorImageInfo(_deviceResources->asyncUpdateInfo.bumpTex->get(), _deviceResources->asyncUpdateInfo.trilinearSampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));

	_deviceResources->device->updateDescriptorSets(writeDescInfo, ARRAY_SIZE(writeDescInfo), nullptr, 0);
}

/// <summary>Loads the textures required for this training course.</summary>
/// <returns>return true if no error occurred.</returns>
void VulkanMultithreading::createImageSamplerDescriptorSets()
{
	_deviceResources->texDescSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->texLayout);
	// create the bilinear sampler
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.minFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	_deviceResources->asyncUpdateInfo.bilinearSampler = _deviceResources->device->createSampler(samplerInfo);

	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_NEAREST;
	_deviceResources->asyncUpdateInfo.trilinearSampler = _deviceResources->device->createSampler(samplerInfo);
}

void VulkanMultithreading::createUbo()
{
	const uint32_t swapchainLength = _deviceResources->swapchain->getSwapchainLength();
	pvrvk::WriteDescriptorSet descUpdate[pvrvk::FrameworkCaps::MaxSwapChains];
	{
		pvr::utils::StructuredMemoryDescription desc;
		desc.addElement("MVPMatrix", pvr::GpuDatatypes::mat4x4);
		desc.addElement("LightDirModel", pvr::GpuDatatypes::vec3);

		_deviceResources->structuredMemoryView.initDynamic(desc, swapchainLength, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));
		_deviceResources->ubo = pvr::utils::createBuffer(_deviceResources->device,
			pvrvk::BufferCreateInfo(_deviceResources->structuredMemoryView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT), pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT,
			_deviceResources->vmaAllocator, pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		_deviceResources->structuredMemoryView.pointToMappedMemory(_deviceResources->ubo->getDeviceMemory()->getMappedData());
	}

	for (uint32_t i = 0; i < swapchainLength; ++i)
	{
		_deviceResources->uboDescSet[i] = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->uboLayoutDynamic);
		descUpdate[i]
			.set(pvrvk::DescriptorType::e_UNIFORM_BUFFER, _deviceResources->uboDescSet[i])
			.setBufferInfo(0,
				pvrvk::DescriptorBufferInfo(
					_deviceResources->ubo, _deviceResources->structuredMemoryView.getDynamicSliceOffset(i), _deviceResources->structuredMemoryView.getDynamicSliceSize()));
	}

	_deviceResources->device->updateDescriptorSets(descUpdate, swapchainLength, nullptr, 0);
}

/// <summary>Loads and compiles the shaders and create a pipeline.</summary>
/// <returns>Return true if no error occurred.</returns>
void VulkanMultithreading::loadPipeline()
{
	//--- create the texture-sampler descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo
			.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT) /*binding 0*/
			.setBinding(1, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT); /*binding 1*/
		_deviceResources->texLayout = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the ubo descriptor set layout
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
		descSetLayoutInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->uboLayoutDynamic = _deviceResources->device->createDescriptorSetLayout(descSetLayoutInfo);
	}

	//--- create the pipeline layout
	{
		pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
		pipeLayoutInfo
			.addDescSetLayout(_deviceResources->texLayout) /*set 0*/
			.addDescSetLayout(_deviceResources->uboLayoutDynamic); /*set 1*/
		_deviceResources->pipelayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
	}
	pvrvk::GraphicsPipelineCreateInfo pipeInfo;
	pipeInfo.rasterizer.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	pipeInfo.colorBlend.setAttachmentState(0, pvrvk::PipelineColorBlendAttachmentState());

	std::unique_ptr<pvr::Stream> vertSource = getAssetStream(VertShaderSrcFile);
	std::unique_ptr<pvr::Stream> fragSource = getAssetStream(FragShaderSrcFile);

	pipeInfo.vertexShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(vertSource->readToEnd<uint32_t>())));
	pipeInfo.fragmentShader.setShader(_deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(fragSource->readToEnd<uint32_t>())));

	const pvr::assets::Mesh& mesh = _scene->getMesh(0);
	pipeInfo.inputAssembler.setPrimitiveTopology(pvr::utils::convertToPVRVk(mesh.getPrimitiveType()));
	pipeInfo.pipelineLayout = _deviceResources->pipelayout;
	pipeInfo.renderPass = _deviceResources->onScreenFramebuffer[0]->getRenderPass();
	pipeInfo.subpass = 0;
	// Enable z-buffer test. We are using a projection matrix optimized for a floating point depth buffer,
	// so the depth test and clear value need to be inverted (1 becomes near, 0 becomes far).
	pipeInfo.depthStencil.enableDepthTest(true);
	pipeInfo.depthStencil.setDepthCompareFunc(pvrvk::CompareOp::e_LESS);
	pipeInfo.depthStencil.enableDepthWrite(true);
	pvr::utils::populateInputAssemblyFromMesh(mesh, VertexAttribBindings, sizeof(VertexAttribBindings) / sizeof(VertexAttribBindings[0]), pipeInfo.vertexInput, pipeInfo.inputAssembler);

	pvr::utils::populateViewportStateCreateInfo(_deviceResources->onScreenFramebuffer[0], pipeInfo.viewport);
	_deviceResources->pipe = _deviceResources->device->createGraphicsPipeline(pipeInfo, _deviceResources->pipelineCache);
}

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it (e.g. external modules, loading meshes, etc.)
/// If the rendering context is lost, initApplication() will not be called again.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanMultithreading::initApplication()
{
	// Load the _scene
	_scene = pvr::assets::loadModel(*this, SceneFile);
	_angleY = 0.0f;
	_frameId = 0;
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by PVRShell once per run, just before exiting the program.
///	If the rendering context is lost, quitApplication() will not be called.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanMultithreading::quitApplication()
{
	_scene.reset();
	return pvr::Result::Success;
}

void DiffuseTextureDoneCallback(pvr::utils::AsyncApiTexture tex)

{
	// We have set the "callbackBeforeSignal" to "true", which means we should NOT call GET before this function returns!
	if (tex->isSuccessful())
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		Log(LogLevel::Information, "ASYNCUPLOADER: Diffuse texture uploading completed successfully.");
	}
	else
	{
		Log(LogLevel::Information, "ASYNCUPLOADER: ERROR uploading normal texture. You can handle this information in your applications.");
	}
}

void NormalTextureDoneCallback(pvr::utils::AsyncApiTexture tex)
{
	// We have set the "callbackBeforeSignal" to "true", which means we should NOT call GET before this function returns!
	if (tex->isSuccessful())
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		Log(LogLevel::Information, "ASYNCUPLOADER: Normal texture uploading has been completed.");
	}
	else
	{
		Log(LogLevel::Information,
			"ASYNCUPLOADER: ERROR uploading normal texture. You can handle this "
			"information in your applications.");
	}
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change in the rendering context.
/// Used to initialize variables that are dependent on the rendering context (e.g. textures, vertex buffers, etc.).</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanMultithreading::initView()
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

	// look for 2 queues one support Graphics and present operation and the second one with transfer operation
	pvr::utils::QueuePopulateInfo queuePopulateInfo = {
		pvrvk::QueueFlags::e_GRAPHICS_BIT,
		surface,
	};
	pvr::utils::QueueAccessInfo queueAccessInfo;
	// create the Logical device
	_deviceResources->device = pvr::utils::createDeviceAndQueues(_deviceResources->instance->getPhysicalDevice(0), &queuePopulateInfo, 1, &queueAccessInfo);

	// Get the queues
	_deviceResources->queue = _deviceResources->device->getQueue(queueAccessInfo.familyId, queueAccessInfo.queueId);

	_deviceResources->vmaAllocator = pvr::utils::vma::createAllocator(pvr::utils::vma::AllocatorCreateInfo(_deviceResources->device));

	// Create the command pool & Descriptor pool
	_deviceResources->commandPool = _deviceResources->device->createCommandPool(
		pvrvk::CommandPoolCreateInfo(_deviceResources->queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 16)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 16)
																						  .setMaxDescriptorSets(16));

	// create a new command pool for image uploading and upload the images in separate thread
	_deviceResources->uploader.init(_deviceResources->device, _deviceResources->queue, &_hostMutex);

	_deviceResources->asyncUpdateInfo.diffuseTex = _deviceResources->uploader.uploadTextureAsync(
		_deviceResources->loader.loadTextureAsync("Marble.pvr", this, pvr::TextureFileFormat::PVR), true, &DiffuseTextureDoneCallback, true);

	_deviceResources->asyncUpdateInfo.bumpTex = _deviceResources->uploader.uploadTextureAsync(
		_deviceResources->loader.loadTextureAsync("MarbleNormalMap.pvr", this, pvr::TextureFileFormat::PVR), true, &NormalTextureDoneCallback, true);

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->instance->getPhysicalDevice(0)->getSurfaceCapabilities(surface);

	// validate the supported swapchain image usage
	pvrvk::ImageUsageFlags swapchainImageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	if (pvr::utils::isImageUsageSupportedBySurface(surfaceCapabilities, pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT))
	{ swapchainImageUsage |= pvrvk::ImageUsageFlags::e_TRANSFER_SRC_BIT; } // Create the swapchain image and depth-stencil image
	auto swapChainCreateOutput = pvr::utils::createSwapchainRenderpassFramebuffers(_deviceResources->device, surface, getDisplayAttributes(),
		pvr::utils::CreateSwapchainParameters().setAllocator(_deviceResources->vmaAllocator).setColorImageUsageFlags(swapchainImageUsage));

	_deviceResources->swapchain = swapChainCreateOutput.swapchain;
	_deviceResources->onScreenFramebuffer = swapChainCreateOutput.framebuffer;

	// Create the pipeline cache
	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache();

	// load the pipeline
	loadPipeline();
	createUbo();

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquiredSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);

		_deviceResources->loadingTextCmdBuffer[i] = _deviceResources->commandPool->allocateCommandBuffer();
		_deviceResources->cmdBuffers[i] = _deviceResources->commandPool->allocateCommandBuffer();
	}

	// load the vbo and ibo data
	_deviceResources->cmdBuffers[0]->begin();
	bool requiresCommandBufferSubmission = false;
	pvr::utils::appendSingleBuffersFromModel(_deviceResources->device, *_scene, _deviceResources->vbos, _deviceResources->ibos, _deviceResources->cmdBuffers[0],
		requiresCommandBufferSubmission, _deviceResources->vmaAllocator);

	_deviceResources->cmdBuffers[0]->end();

	if (requiresCommandBufferSubmission)
	{
		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &_deviceResources->cmdBuffers[0];
		submitInfo.numCommandBuffers = 1;

		// submit the queue and wait for it to become idle
		_deviceResources->queue->submit(&submitInfo, 1);
		_deviceResources->queue->waitIdle();
	}

	//  Initialize UIRenderer
	_deviceResources->uiRenderer.init(getWidth(), getHeight(), isFullScreen(), _deviceResources->onScreenFramebuffer[0]->getRenderPass(), 0,
		getBackBufferColorspace() == pvr::ColorSpace::sRGB, _deviceResources->commandPool, _deviceResources->queue);

	_deviceResources->uiRenderer.getDefaultTitle()->setText("Multithreading");
	_deviceResources->uiRenderer.getDefaultTitle()->commitUpdates();
	glm::vec3 from, to, up;
	float fov;
	_scene->getCameraProperties(0, fov, from, to, up);

	// Is the screen rotated
	bool bRotate = this->isScreenRotated();

	//  Calculate the projection and rotate it by 90 degree if the screen is rotated.
	_viewProj = (bRotate ? pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(this->getHeight()), static_cast<float>(this->getWidth()),
							   _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar(), glm::pi<float>() * .5f)
						 : pvr::math::perspectiveFov(pvr::Api::Vulkan, fov, static_cast<float>(this->getWidth()), static_cast<float>(this->getHeight()),
							   _scene->getCamera(0).getNear(), _scene->getCamera(0).getFar()));

	_viewProj = _viewProj * glm::lookAt(from, to, up);
	recordLoadingCommandBuffer();
	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by PVRShell when the application quits or before a change in the rendering context.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanMultithreading::releaseView()
{
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Return Result::Success if no error occurred.</returns>
pvr::Result VulkanMultithreading::renderFrame()
{
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquiredSemaphores[_frameId]);

	const uint32_t swapchainIndex = _deviceResources->swapchain->getSwapchainIndex();

	_deviceResources->perFrameResourcesFences[swapchainIndex]->wait();
	_deviceResources->perFrameResourcesFences[swapchainIndex]->reset();

	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags waitDstStageMask = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitDstStageMask = &waitDstStageMask;
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquiredSemaphores[_frameId];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	submitInfo.numSignalSemaphores = 1;

	if (!_loadingDone)
	{
		if (_deviceResources->asyncUpdateInfo.bumpTex->isComplete() && _deviceResources->asyncUpdateInfo.diffuseTex->isComplete())
		{
			createImageSamplerDescriptorSets();
			updateTextureDescriptorSet();
			recordMainCommandBuffer();
			_loadingDone = true;
		}
	}
	if (!_loadingDone)
	{
		static float f = 0;
		f += getFrameTime() * .0005f;
		if (f > glm::pi<float>() * .5f) { f = 0; }
		_deviceResources->loadingText[swapchainIndex]->setColor(1.0f, 1.0f, 1.0f, f + .01f);
		_deviceResources->loadingText[swapchainIndex]->setScale(sin(f) * 3.f, sin(f) * 3.f);
		_deviceResources->loadingText[swapchainIndex]->commitUpdates();

		submitInfo.commandBuffers = &_deviceResources->loadingTextCmdBuffer[swapchainIndex];
	}

	if (_loadingDone)
	{
		// Calculate the model matrix
		glm::mat4 mModel = glm::rotate(_angleY, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(1.8f));
		_angleY += -RotateY * 0.05f * getFrameTime();

		// Set light Direction in model space
		//  The inverse of a rotation matrix is the transposed matrix
		//  Because of v * M = transpose(M) * v, this means:
		//  v * R == inverse(R) * v
		//  So we don't have to actually invert or transpose the matrix
		//  to transform back from world space to model space

		// update the ubo
		{
			UboPerMeshData srcWrite;
			srcWrite.lightDirModel = glm::vec3(LightDir * mModel);
			srcWrite.mvpMtx = _viewProj * mModel * _scene->getWorldMatrix(_scene->getNode(0).getObjectId());

			uint32_t currentDynamicSlice = swapchainIndex * _scene->getNumMeshNodes();
			_deviceResources->structuredMemoryView.getElement(0, 0, currentDynamicSlice).setValue(srcWrite.mvpMtx);
			_deviceResources->structuredMemoryView.getElement(1, 0, currentDynamicSlice).setValue(srcWrite.lightDirModel);

			// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
			if (static_cast<uint32_t>(_deviceResources->ubo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
			{
				_deviceResources->ubo->getDeviceMemory()->flushRange(_deviceResources->structuredMemoryView.getDynamicSliceOffset(swapchainIndex * _scene->getNumMeshNodes()),
					_deviceResources->structuredMemoryView.getDynamicSliceSize() * _scene->getNumMeshNodes());
			}
		}
		submitInfo.commandBuffers = &_deviceResources->cmdBuffers[swapchainIndex];
	}

	{
		std::lock_guard<pvr::async::Mutex> lock(_hostMutex);
		// submit
		_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[swapchainIndex]);
	}

	if (this->shouldTakeScreenshot())
	{
		pvr::utils::takeScreenshot(_deviceResources->queue, _deviceResources->commandPool, _deviceResources->swapchain, swapchainIndex, getScreenshotFileName(),
			_deviceResources->vmaAllocator, _deviceResources->vmaAllocator);
	}

	// present
	pvrvk::PresentInfo present;
	present.swapchains = &_deviceResources->swapchain;
	present.imageIndices = &swapchainIndex;
	present.numSwapchains = 1;
	present.waitSemaphores = &_deviceResources->presentationSemaphores[_frameId];
	present.numWaitSemaphores = 1;
	_deviceResources->queue->present(present);

	_frameId = (_frameId + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/// <summary>Draws a assets::Mesh after the model view matrix has been set and the material prepared.</summary>
/// <param>nodeIndex Node index of the mesh to draw.</param>
void VulkanMultithreading::drawMesh(pvrvk::CommandBuffer& cmdBuffers, int nodeIndex)
{
	uint32_t meshId = _scene->getNode(nodeIndex).getObjectId();
	const pvr::assets::Mesh& mesh = _scene->getMesh(meshId);

	// bind the VBO for the mesh
	cmdBuffers->bindVertexBuffer(_deviceResources->vbos[meshId], 0, 0);

	//  The geometry can be exported in 4 ways:
	//  - Indexed Triangle list
	//  - Non-Indexed Triangle list
	//  - Indexed Triangle strips
	//  - Non-Indexed Triangle strips
	if (mesh.getNumStrips() == 0)
	{
		// Indexed Triangle list
		if (_deviceResources->ibos[meshId])
		{
			cmdBuffers->bindIndexBuffer(_deviceResources->ibos[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
			cmdBuffers->drawIndexed(0, mesh.getNumFaces() * 3, 0, 0, 1);
		}
		else
		{
			// Non-Indexed Triangle list
			cmdBuffers->draw(0, mesh.getNumFaces() * 3, 0, 1);
		}
	}
	else
	{
		for (uint32_t i = 0; i < mesh.getNumStrips(); ++i)
		{
			int offset = 0;
			if (_deviceResources->ibos[meshId])
			{
				// Indexed Triangle strips
				cmdBuffers->bindIndexBuffer(_deviceResources->ibos[meshId], 0, pvr::utils::convertToPVRVk(mesh.getFaces().getDataType()));
				cmdBuffers->drawIndexed(0, mesh.getStripLength(i) + 2, offset * 2, 0, 1);
			}
			else
			{
				// Non-Indexed Triangle strips
				cmdBuffers->draw(0, mesh.getStripLength(i) + 2, 0, 1);
			}
			offset += mesh.getStripLength(i) + 2;
		}
	}
}

/// <summary>Pre record the commands.</summary>
void VulkanMultithreading::recordMainCommandBuffer()
{
	const pvrvk::ClearValue clearValues[] = { pvrvk::ClearValue(0.0f, 0.40f, .39f, 1.f), pvrvk::ClearValue(1.f, 0u) };
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::CommandBuffer& cmdBuffers = _deviceResources->cmdBuffers[i];
		cmdBuffers->begin();
		cmdBuffers->beginRenderPass(_deviceResources->onScreenFramebuffer[i], pvrvk::Rect2D(0, 0, getWidth(), getHeight()), true, clearValues, ARRAY_SIZE(clearValues));
		// enqueue the static states which wont be changed through out the frame
		cmdBuffers->bindPipeline(_deviceResources->pipe);
		cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelayout, 0, _deviceResources->texDescSet, 0);
		cmdBuffers->bindDescriptorSet(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelayout, 1, _deviceResources->uboDescSet[i]);
		drawMesh(cmdBuffers, 0);

		// record the uirenderer commands
		_deviceResources->uiRenderer.beginRendering(cmdBuffers);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->uiRenderer.endRendering();
		cmdBuffers->endRenderPass();
		cmdBuffers->end();
	}
}

/// <summary>Pre record the commands.</summary>
void VulkanMultithreading::recordLoadingCommandBuffer()
{
	const pvrvk::ClearValue clearColor[2] = { pvrvk::ClearValue(0.0f, 0.40f, .39f, 1.f), pvrvk::ClearValue(1.f, 0u) };

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::CommandBuffer& cmdBuffers = _deviceResources->loadingTextCmdBuffer[i];
		cmdBuffers->begin();

		cmdBuffers->beginRenderPass(_deviceResources->onScreenFramebuffer[i], true, clearColor, ARRAY_SIZE(clearColor));

		_deviceResources->loadingText[i] = _deviceResources->uiRenderer.createText("Loading...");
		_deviceResources->loadingText[i]->commitUpdates();

		// record the uirenderer commands
		_deviceResources->uiRenderer.beginRendering(cmdBuffers);
		_deviceResources->uiRenderer.getDefaultTitle()->render();
		_deviceResources->uiRenderer.getSdkLogo()->render();
		_deviceResources->loadingText[i]->render();
		_deviceResources->uiRenderer.endRendering();

		cmdBuffers->endRenderPass();
		cmdBuffers->end();
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanMultithreading>(); }
