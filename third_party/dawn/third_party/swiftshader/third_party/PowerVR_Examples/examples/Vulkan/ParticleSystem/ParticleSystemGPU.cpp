/*!
\brief Particle system implemented using GPU Compute Shaders.
\file ParticleSystemGPU.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "ParticleSystemGPU.h"
#include "PVRVk/ComputePipelineVk.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/PipelineLayoutVk.h"
#include "PVRVk/QueueVk.h"

ParticleSystemGPU::ParticleSystemGPU(pvr::Shell& assetLoader)
	: computeShaderSrcFile("ParticleSolver.csh"), gravity(0.0f), numParticles(0), workgroupSize(32), assetProvider(assetLoader)
{
	memset(&particleConfigData, 0, sizeof(ParticleConfig));
}

ParticleSystemGPU::~ParticleSystemGPU()
{
	if (device) { device->waitIdle(); }
	for (uint32_t i = 0; i < MultiBuffers; ++i)
	{
		if (perStepResourcesFences[i]) perStepResourcesFences[i]->wait();
	}
}

void ParticleSystemGPU::init(uint32_t inMaxParticles, const std::vector<Sphere> spheres, pvrvk::Device& inDevice, pvrvk::Queue& inQueue, pvrvk::DescriptorPool& descriptorPool,
	pvr::utils::vma::Allocator& inAllocator, pvrvk::PipelineCache& inPipelineCache, const std::vector<pvrvk::Semaphore> waitSemaphores)
{
	this->device = inDevice;
	this->queue = inQueue;

	// Verify that the given queue supports compute capabilities
	if (static_cast<uint32_t>(queue->getFlags() & pvrvk::QueueFlags::e_COMPUTE_BIT) == 0) { Log(LogLevel::Error, "Queue must support Compute capabilities"); }

	this->commandPool = device->createCommandPool(pvrvk::CommandPoolCreateInfo(queue->getFamilyIndex(), pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
	this->maxParticles = inMaxParticles;
	this->allocator = inAllocator;
	this->pipelineCache = inPipelineCache;
	this->externalWaitSemaphores = waitSemaphores;
	this->externalWaitSemaphoreIndices.resize(this->externalWaitSemaphores.size());

	emitterSet = false;
	gravitySet = false;
	numParticlesSet = false;

	currentResourceIndex = 0;
	previousResourceIndex = 0;
	stepCount = 0;
	externalWaitFrameIndex = 0;
	currentExternalWaitFrameIndex = 0;

	createDescriptorSetLayout();
	createComputePipeline();
	createCommandBuffers();
	setCollisionSpheres(spheres);

	// Create the particle system buffers
	// The particle system buffers will be allocated large enough for the maximum number of particles
	particleSystemBufferSliceSize = sizeof(Particle) * maxParticles;
	for (uint8_t i = 0; i < MultiBuffers; ++i)
	{
		particleSystemBuffers[i] = pvr::utils::createBuffer(device,
			pvrvk::BufferCreateInfo(static_cast<VkDeviceSize>(particleSystemBufferSliceSize),
				pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_STORAGE_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT),
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, allocator, pvr::utils::vma::AllocationCreateFlags::e_NONE);
	}

	// Create a configuration buffer for the particle system to be used for controlling the particle system behaviour
	{
		particleConfigUboBufferView.initDynamic(ParticleConfigViewMapping, MultiBuffers, pvr::BufferUsageFlags::UniformBuffer,
			static_cast<uint32_t>(device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment()));

		particleConfigUbo = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(particleConfigUboBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
			pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
			pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, allocator,
			pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

		particleConfigUboBufferView.pointToMappedMemory(particleConfigUbo->getDeviceMemory()->getMappedData());
	}

	// Create the particle system descriptor sets and update them using the previously allocated resources
	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;
	for (uint8_t i = 0; i < MultiBuffers; ++i)
	{
		descSets[i] = descriptorPool->allocateDescriptorSet(descriptorSetLayout);

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descSets[i], BufferBindingPoint::SPHERES_UBO_BINDING_INDEX)
									.setBufferInfo(0, pvrvk::DescriptorBufferInfo(collisonSpheresUbo, 0, collisonSpheresUboBufferView.getDynamicSliceSize())));

		writeDescSets.push_back(
			pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER, descSets[i], BufferBindingPoint::PARTICLE_CONFIG_UBO_BINDING_INDEX)
				.setBufferInfo(
					0, pvrvk::DescriptorBufferInfo(particleConfigUbo, particleConfigUboBufferView.getDynamicSliceOffset(i), particleConfigUboBufferView.getDynamicSliceSize())));

		uint32_t inputIndex = i % MultiBuffers;
		uint32_t outputIndex = (i + 1) % MultiBuffers;

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, descSets[i], BufferBindingPoint::PARTICLES_SSBO_BINDING_INDEX_IN)
									.setBufferInfo(0, pvrvk::DescriptorBufferInfo(particleSystemBuffers[inputIndex], 0, particleSystemBufferSliceSize)));

		writeDescSets.push_back(pvrvk::WriteDescriptorSet(pvrvk::DescriptorType::e_STORAGE_BUFFER, descSets[i], BufferBindingPoint::PARTICLES_SSBO_BINDING_INDEX_OUT)
									.setBufferInfo(0, pvrvk::DescriptorBufferInfo(particleSystemBuffers[outputIndex], 0, particleSystemBufferSliceSize)));
	}
	device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);

	stagingBuffer = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(particleSystemBuffers[0]->getSize(), pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, allocator,
		pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	stagingFence = device->createFence();
	commandStaging = commandPool->allocateCommandBuffer();

	for (uint8_t i = 0; i < MultiBuffers; ++i)
	{
		particleSystemSemaphores[i] = device->createSemaphore();
		outputSemaphores[i] = device->createSemaphore();
		perStepResourcesFences[i] = device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}
}

void ParticleSystemGPU::createDescriptorSetLayout()
{
	pvrvk::DescriptorSetLayoutCreateInfo descSetLayoutInfo;
	descSetLayoutInfo.setBinding(BufferBindingPoint::SPHERES_UBO_BINDING_INDEX, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT)
		.setBinding(BufferBindingPoint::PARTICLE_CONFIG_UBO_BINDING_INDEX, pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT)
		.setBinding(BufferBindingPoint::PARTICLES_SSBO_BINDING_INDEX_IN, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT)
		.setBinding(BufferBindingPoint::PARTICLES_SSBO_BINDING_INDEX_OUT, pvrvk::DescriptorType::e_STORAGE_BUFFER, 1, pvrvk::ShaderStageFlags::e_COMPUTE_BIT);
	descriptorSetLayout = device->createDescriptorSetLayout(descSetLayoutInfo);

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.setDescSetLayout(0, descriptorSetLayout);
	pipelineLayout = device->createPipelineLayout(pipeLayoutInfo);
}

void ParticleSystemGPU::createComputePipeline()
{
	pvrvk::ComputePipelineCreateInfo pipeCreateInfo;
	pvrvk::ShaderModule shader = device->createShaderModule(pvrvk::ShaderModuleCreateInfo(assetProvider.getAssetStream(ComputeShaderFileName)->readToEnd<uint32_t>()));
	pipeCreateInfo.computeShader.setShader(shader);
	pipeCreateInfo.pipelineLayout = pipelineLayout;
	pipeline = device->createComputePipeline(pipeCreateInfo, pipelineCache);
}

void ParticleSystemGPU::updateTime(float dt)
{
	dt *= 0.001f;
	particleConfigData.fDt = dt;
	particleConfigData.fTotalTime += dt;

	particleConfigData.updateBufferView(particleConfigUboBufferView, particleConfigUbo, currentResourceIndex);
}

void ParticleSystemGPU::setNumberOfParticles(uint32_t numParticles_)
{
	assertion(numParticles_ <= this->maxParticles);

	this->numParticles = numParticles_;

	// Default initialise the particles in the staging buffer
	Particle* tmpData = (Particle*)stagingBuffer->getDeviceMemory()->getMappedData();
	for (uint32_t i = 0; i < numParticles; ++i)
	{
		tmpData[i].vPosition.x = 0.0f;
		tmpData[i].vPosition.y = 0.0f;
		tmpData[i].vPosition.z = 1.0f;
		tmpData[i].vVelocity = glm::vec3(0.0f);
		tmpData[i].fTimeToLive = pvr::randomrange(0.0f, 1.5f);
	}

	// Flush the memory if required
	if ((stagingBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
	{ stagingBuffer->getDeviceMemory()->flushRange(0, stagingBuffer->getSize()); }

	// Effectively resets the particle system buffers ready for simulation of numParticles
	// First fill the particle system buffers with 0's for the entire buffer
	// Then copy from the staging buffer to the particle system buffers
	commandStaging->begin();
	for (uint32_t i = 0; i < MultiBuffers; ++i)
	{
		commandStaging->fillBuffer(particleSystemBuffers[i], 0, 0, particleSystemBuffers[i]->getSize());
		pvrvk::BufferCopy bufferCopy = pvrvk::BufferCopy(0, 0, sizeof(Particle) * numParticles);
		// Second copy the staging buffer contents up into the particle system buffers
		commandStaging->copyBuffer(stagingBuffer, particleSystemBuffers[i], 1, &bufferCopy);
	}
	commandStaging->end();

	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &commandStaging;
	submitInfo.numCommandBuffers = 1;

	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_ALL_BITS;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;
	queue->submit(&submitInfo, 1, stagingFence);
	stagingFence->wait();
	stagingFence->reset();

	// Re-records commands for numParticles
	recordCommandBuffers();

	numParticlesSet = true;
}

void ParticleSystemGPU::setEmitter(const Emitter& emitter)
{
	particleConfigData.emitter = emitter;
	emitterSet = true;
}

void ParticleSystemGPU::setGravity(const glm::vec3& g)
{
	particleConfigData.vG = g;
	gravitySet = true;
}

void ParticleSystemGPU::setCollisionSpheres(const std::vector<Sphere> spheres)
{
	collisonSpheresUboBufferView.init(pvr::utils::StructuredMemoryDescription("SphereBuffer", 1, { { "SphereArray", 8, pvr::GpuDatatypes::vec4 } }));

	collisonSpheresUbo = pvr::utils::createBuffer(device, pvrvk::BufferCreateInfo(collisonSpheresUboBufferView.getSize(), pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT),
		pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT,
		pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT | pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT, allocator,
		pvr::utils::vma::AllocationCreateFlags::e_MAPPED_BIT);

	collisonSpheresUboBufferView.pointToMappedMemory(collisonSpheresUbo->getDeviceMemory()->getMappedData());
	for (uint32_t i = 0; i < spheres.size(); ++i)
	{ collisonSpheresUboBufferView.getElement(SphereViewElements::PositionRadius, i).setValue(glm::vec4(spheres[i].vPosition, spheres[i].fRadius)); }
}

void ParticleSystemGPU::recordCommandBuffers()
{
	for (uint8_t i = 0; i < MultiBuffers; ++i)
	{
		computeCommandBuffers[i]->reset();
		computeCommandBuffers[i]->begin();
		computeCommandBuffers[i]->bindPipeline(pipeline);
		computeCommandBuffers[i]->bindDescriptorSets(pvrvk::PipelineBindPoint::e_COMPUTE, pipelineLayout, 0, &descSets[i], 1);
		computeCommandBuffers[i]->dispatch(numParticles / workgroupSize, 1, 1);
		computeCommandBuffers[i]->end();

		mainCommandBuffers[i]->begin();
		mainCommandBuffers[i]->executeCommands(computeCommandBuffers[i]);
		mainCommandBuffers[i]->end();
	}
}

void ParticleSystemGPU::createCommandBuffers()
{
	for (uint8_t i = 0; i < MultiBuffers; ++i)
	{
		computeCommandBuffers[i] = commandPool->allocateSecondaryCommandBuffer();
		mainCommandBuffers[i] = commandPool->allocateCommandBuffer();
	}
}

const pvrvk::Semaphore& ParticleSystemGPU::step(uint32_t waitSemaphoreIndex)
{
	assertion(emitterSet);
	assertion(gravitySet);
	assertion(numParticlesSet);

	assertion(waitSemaphoreIndex < externalWaitSemaphores.size());
	// Handle out of order steps
	externalWaitSemaphoreIndices[externalWaitFrameIndex] = waitSemaphoreIndex;

	// Wait for and reset the fence for set of current resources to be free from perStepResourcesFences.size() steps ago
	perStepResourcesFences[currentResourceIndex]->wait();
	perStepResourcesFences[currentResourceIndex]->reset();

	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags[2];

	submitInfo.commandBuffers = &mainCommandBuffers[currentResourceIndex];
	submitInfo.numCommandBuffers = 1;

	pvrvk::Semaphore waitSemaphores[2];

	// When the particle system has advanced at least once we add a semaphore on the previous particle system step to ensure prior to completion
	if (stepCount > 0)
	{
		waitSemaphores[0] = particleSystemSemaphores[previousResourceIndex];
		submitInfo.numWaitSemaphores++;
		pipeWaitStageFlags[0] = pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT;
	}

	// Once the particle system has advanced at least MultiBuffers times we add another semaphore using the waitSemaphoreIndex from externalWaitSemaphores.size() steps ago
	if (stepCount >= MultiBuffers)
	{
		waitSemaphores[1] = externalWaitSemaphores[externalWaitSemaphoreIndices[currentExternalWaitFrameIndex]];
		submitInfo.numWaitSemaphores++;
		pipeWaitStageFlags[1] = pvrvk::PipelineStageFlags::e_COMPUTE_SHADER_BIT;
	}
	submitInfo.waitSemaphores = waitSemaphores;
	submitInfo.waitDstStageMask = pipeWaitStageFlags;

	// The particle system will signal two semaphores:
	// 1. An internal semaphore used to serial particle system steps
	// 2. An external semaphore used by external commands used to ensure that the particle system step has fully completed
	pvrvk::Semaphore signalSemaphores[] = { particleSystemSemaphores[currentResourceIndex], outputSemaphores[currentResourceIndex] };
	submitInfo.signalSemaphores = signalSemaphores;
	submitInfo.numSignalSemaphores = 2;
	queue->submit(&submitInfo, 1, perStepResourcesFences[currentResourceIndex]);

	// Update current/previous resource indices
	previousResourceIndex = currentResourceIndex;
	currentResourceIndex = (currentResourceIndex + 1) % MultiBuffers;

	// Update the external wait semaphore indices
	if (stepCount < MultiBuffers) { stepCount++; }
	else
	{
		currentExternalWaitFrameIndex = (currentExternalWaitFrameIndex + 1) % externalWaitSemaphores.size();
	}

	externalWaitFrameIndex = (externalWaitFrameIndex + 1) % externalWaitSemaphores.size();

	// Return the external semaphore based on the current step call. Uses previousResourceIndex because current/previous resource indices have already been updated
	return outputSemaphores[previousResourceIndex];
}
