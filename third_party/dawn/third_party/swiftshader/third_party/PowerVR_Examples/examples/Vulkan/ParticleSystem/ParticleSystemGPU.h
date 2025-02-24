/*!*********************************************************************************************************************
\File         ParticleSystemGPU.h
\Title        ParticleSystemGPU
\Author       PowerVR by Imagination, Developer Technology Team
\Copyright    Copyright (c) Imagination Technologies Limited.
\Description  Particle system implemented using direct manipulation of the VBOs in order to implement zero-copy operations on the GPU.
***********************************************************************************************************************/
#pragma once

#include "PVRShell/PVRShell.h"
#include "PVRCore/strings/StringHash.h"
#include "PVRUtils/PVRUtilsVk.h"

// Compute shader kernel to be used to update the particle system each frame
const char* const ComputeShaderFileName = "ParticleSolver.csh.spv";

// The particle structure will be kept packed. We will have to be careful with strides
struct Particle
{
	glm::vec3 vPosition; // vec3
	float _pad;
	glm::vec3 vVelocity; // vec4.xyz
	float fTimeToLive; // vec4/w
}; // SIZE:32 bytes

const pvr::utils::StructuredMemoryDescription ParticleViewMapping(
	"ParticlesBuffer", 1, { { "vPosition", pvr::GpuDatatypes::vec3 }, { "vVelocity", pvr::GpuDatatypes::vec3 }, { "fTimeToLive", pvr::GpuDatatypes::Float } });

namespace ParticleViewElements {
enum Enum
{
	Position,
	Velocity,
	TimeToLive
};
}

// All the following will all be used in uniforms/ssbos, so we will mimic the alignment of std140 glsl
// layout spec in order to make their use simpler
struct Sphere
{
	glm::vec3 vPosition; // vec4: xyz
	float fRadius; // vec4: w
	Sphere(const glm::vec3& pos, float radius) : vPosition(pos), fRadius(radius) {}
};

namespace SphereViewElements {
enum Enum
{
	PositionRadius
};
}

struct Emitter
{
	glm::mat4 mTransformation; // mat4
	float fHeight; // float
	float fRadius; // float
	Emitter(const glm::mat4& trans, float height, float radius) : mTransformation(trans), fHeight(height), fRadius(radius) {}
	Emitter() : mTransformation(1.0f), fHeight(0.0f), fRadius(0.0f) {}
};

const pvr::utils::StructuredMemoryDescription ParticleConfigViewMapping("ParticleConfig", 1,
	{
		// Emitter
		{ "mTransformation", pvr::GpuDatatypes::mat4x4 },
		{ "fHeight", pvr::GpuDatatypes::Float },
		{ "fRadius", pvr::GpuDatatypes::Float },
		{ "vG", pvr::GpuDatatypes::vec3 },
		{ "fDt", pvr::GpuDatatypes::Float },
		{ "fTotalTime", pvr::GpuDatatypes::Float },
	});

namespace ParticleConfigViewElements {
enum Enum
{
	EmitterTransform,
	EmitterHeight,
	EmitterRadius,
	Gravity,
	DeltaTime,
	TotalTime
};
}

struct ParticleConfig
{
	Emitter emitter;
	glm::vec3 vG;
	float fDt;
	float fTotalTime;
	// size of the 1st element of an array, so we must upload
	// enough data for it to be a multiple of vec4(i.e. 4floats/16 bytes : 25->28)
	ParticleConfig() : vG(glm::vec3(0.0f)), fDt(0.0f), fTotalTime(0.0f) {}

	// Update the particle system configuration for the specified version of the particle system
	void updateBufferView(pvr::utils::StructuredBufferView& view, pvrvk::Buffer& buffer, uint32_t index)
	{
		view.getElement(ParticleConfigViewElements::EmitterTransform, 0, index).setValue(emitter.mTransformation);
		view.getElement(ParticleConfigViewElements::EmitterHeight, 0, index).setValue(emitter.fHeight);
		view.getElement(ParticleConfigViewElements::EmitterRadius, 0, index).setValue(emitter.fRadius);
		view.getElement(ParticleConfigViewElements::Gravity, 0, index).setValue(vG);
		view.getElement(ParticleConfigViewElements::DeltaTime, 0, index).setValue(fDt);
		view.getElement(ParticleConfigViewElements::TotalTime, 0, index).setValue(fTotalTime);

		// if the memory property flags used by the buffers' device memory do not contain e_HOST_COHERENT_BIT then we must flush the memory
		if (static_cast<uint32_t>(buffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{ buffer->getDeviceMemory()->flushRange(view.getDynamicSliceOffset(index), view.getDynamicSliceSize()); }
	}
};

class ParticleSystemGPU
{
public:
	ParticleSystemGPU(pvr::Shell& assetLoader);
	~ParticleSystemGPU();

	/// <summary>Initialise the particle system.</summary>
	/// <param name="inMaxParticles">The maximum number of particles the particle system can simulate. The maximum particle count will be used for resource allocation size.</param>
	/// <param name="spheres">A list of spheres which will be used to make more interesting the particle system and can be collided into by particles.</param>
	/// <param name="inDevice">The Vulkan device from which resources will be allocated.</param>
	/// <param name="inQueue">The Vulkan queue to which commands will be submitted.</param>
	/// <param name="descriptorPool">A Vulkan descriptor pool from which descriptors will be allocated.</param>
	/// <param name="inAllocator">A Vulkan allocator from which memory will be allocated.</param>
	/// <param name="inPipelineCache">A pipeline cache used to optimise the creation of pipelines.</param>
	/// <param name="waitSemaphores">A list of semaphores on which the particle system update may wait.</param>
	void init(uint32_t inMaxParticles, const std::vector<Sphere> spheres, pvrvk::Device& inDevice, pvrvk::Queue& inQueue, pvrvk::DescriptorPool& descriptorPool,
		pvr::utils::vma::Allocator& inAllocator, pvrvk::PipelineCache& inPipelineCache, const std::vector<pvrvk::Semaphore> waitSemaphores);

	/// <summary>Sets the current number of particles being simulated by the particle system.</summary>
	/// <param name="numParticles">The current number of particles being simulated by the particle system. Note that this value must be less than the provided maximum number of particles.</param>
	void setNumberOfParticles(uint32_t numParticles);

	/// <summary>Sets the current number of particles being simulated by the particle system.</summary>
	/// <returns>Retrieves the current number of particles being simulated by the particle system.</param>
	uint32_t getNumberOfParticles() const { return numParticles; }

	/// <summary>Sets the emitter used by the particle system.</summary>
	/// <param name="emitter">The emitter for the particle system to use.</param>
	void setEmitter(const Emitter& emitter);

	/// <summary>Sets the gravity used by the particle system.</summary>
	/// <param name="gravity">The gravity used for the particle system simulation.</param>
	void setGravity(const glm::vec3& gravity);

	/// <summary>Advances the simulation by a specified amount.</summary>
	/// <param name="dt">The amount of time to update the simulation by.</param>
	void updateTime(float dt);

	/// <summary>Retrieves the current particle system buffer. The 'current' here refers to retrieving the particle system corresponding to the last step call made.</summary>
	/// <returns>The current particle system buffer.</param>
	const pvrvk::Buffer& getParticleSystemBuffer() const { return particleSystemBuffers[currentResourceIndex]; }

	/// <summary>Advances the particle system simulation by a single step.</summary>
	/// <param name="waitSemaphoreIndex">The index into the array of wait semaphores provided to the particle system at initialisation on which the current step should wait prior
	/// to advancing.</param>
	/// <returns>Returns a semaphore on which any users of the particle system should wait on before making use of the particle system resources.</param>
	const pvrvk::Semaphore& step(uint32_t waitSemaphoreIndex);

private:
	void recordCommandBuffers();
	void createDescriptorSetLayout();
	void createComputePipeline();
	void createCommandBuffers();

	/// <summary>Sets the Spheres used for collision in the particle system simulation.</summary>
	/// <param name="spheres">The spheres for the collisions in the particle system simulation.</param>
	void setCollisionSpheres(const std::vector<Sphere> spheres);

	const static uint8_t MultiBuffers = 2u;

	enum BufferBindingPoint
	{
		SPHERES_UBO_BINDING_INDEX = 0,
		PARTICLE_CONFIG_UBO_BINDING_INDEX = 1,
		PARTICLES_SSBO_BINDING_INDEX_IN = 2,
		PARTICLES_SSBO_BINDING_INDEX_OUT = 3
	};

	// SHADERS
	const char* computeShaderSrcFile;
	pvrvk::ComputePipeline pipeline;
	pvrvk::PipelineLayout pipelineLayout;
	pvrvk::PipelineCache pipelineCache;
	pvrvk::DescriptorSetLayout descriptorSetLayout;
	pvr::utils::vma::Allocator allocator;

	// SIMULATION DATA
	glm::vec3 gravity;
	uint32_t numParticles;
	uint32_t maxParticles;
	uint32_t workgroupSize;
	uint32_t particleSystemBufferSliceSize;
	uint32_t currentResourceIndex;
	uint32_t previousResourceIndex;
	uint32_t stepCount;
	uint32_t currentExternalWaitFrameIndex;
	uint32_t externalWaitFrameIndex;
	ParticleConfig particleConfigData;

	std::vector<uint32_t> externalWaitSemaphoreIndices;
	std::vector<pvrvk::Semaphore> externalWaitSemaphores;

	// OPENGL BUFFER OBJECTS
	pvr::utils::StructuredBufferView collisonSpheresUboBufferView;
	pvrvk::Buffer collisonSpheresUbo;
	pvr::IAssetProvider& assetProvider;

	pvrvk::Buffer stagingBuffer;
	pvrvk::Buffer particleSystemBuffers[MultiBuffers];
	pvrvk::DescriptorSet descSets[MultiBuffers];

	pvr::utils::StructuredBufferView particleConfigUboBufferView;
	pvrvk::Buffer particleConfigUbo;

	pvrvk::CommandBuffer commandStaging;
	pvrvk::Fence stagingFence;
	pvrvk::SecondaryCommandBuffer computeCommandBuffers[MultiBuffers];
	pvrvk::CommandBuffer mainCommandBuffers[MultiBuffers];
	pvrvk::Queue queue;
	pvrvk::CommandPool commandPool;
	pvrvk::Device device;

	pvrvk::Semaphore particleSystemSemaphores[MultiBuffers];
	pvrvk::Semaphore outputSemaphores[MultiBuffers];
	pvrvk::Fence perStepResourcesFences[MultiBuffers];

	bool emitterSet;
	bool gravitySet;
	bool numParticlesSet;

	// Disable copy construction and assignment operator
	ParticleSystemGPU(ParticleSystemGPU&);
	ParticleSystemGPU& operator=(ParticleSystemGPU&);
};
