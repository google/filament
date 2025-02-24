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

#include "Util.hpp"
#include "VulkanTester.hpp"

#include "benchmark/benchmark.h"

#include <cmath>
#include <cstring>
#include <sstream>

// C++ reference implementation for single-threaded 'compute' operations.
template<typename Init, typename Func>
void CppCompute(benchmark::State &state, Init init, Func op)
{
	int64_t numElements = state.range(0);
	float *bufferIn = (float *)malloc(numElements * sizeof(float));
	float *bufferOut = (float *)malloc(numElements * sizeof(float));

	for(int64_t i = 0; i < numElements; i++)
	{
		bufferIn[i] = init(i);
	}

	for(auto _ : state)
	{
		for(int64_t i = 0; i < numElements; i++)
		{
			bufferOut[i] = op(bufferIn[i]);
		}
	}

	free(bufferIn);
	free(bufferOut);
}

float zero(int64_t i)
{
	return 0.0f;
}

float one(int64_t i)
{
	return 1.0f;
}

BENCHMARK_CAPTURE(CppCompute, mov, zero, [](float x) { return x; })->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(CppCompute, sqrt, one, sqrtf)->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(CppCompute, sin, zero, sinf)->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(CppCompute, cos, zero, cosf)->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(CppCompute, exp, zero, expf)->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(CppCompute, log, one, logf)->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond);

class ComputeBenchmark
{
protected:
	ComputeBenchmark()
	{
		tester.initialize();
	}

	VulkanTester tester;
};

// Base class for compute benchmarks that read from an input buffer and write to an
// output buffer of the same length.
class BufferToBufferComputeBenchmark : public ComputeBenchmark
{
public:
	BufferToBufferComputeBenchmark(const benchmark::State &state)
	    : state(state)
	{
		device = tester.getDevice();
	}

	virtual ~BufferToBufferComputeBenchmark()
	{
		device.destroyCommandPool(commandPool);
		device.destroyDescriptorPool(descriptorPool);
		device.destroyPipeline(pipeline);
		device.destroyDescriptorSetLayout(descriptorSetLayout);
		device.destroyBuffer(bufferIn);
		device.destroyBuffer(bufferOut);
		device.freeMemory(deviceMemory);
	}

	void run();

protected:
	void initialize(const std::string &glslShader);

	uint32_t localSizeX = 128;
	uint32_t localSizeY = 1;
	uint32_t localSizeZ = 1;

private:
	const benchmark::State &state;

	// Weak references
	vk::Device device;
	vk::Queue queue;
	vk::CommandBuffer commandBuffer;

	// Owned resources
	vk::CommandPool commandPool;
	vk::DescriptorPool descriptorPool;
	vk::Pipeline pipeline;
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::DeviceMemory deviceMemory;
	vk::Buffer bufferIn;
	vk::Buffer bufferOut;
};

void BufferToBufferComputeBenchmark::initialize(const std::string &glslShader)
{
	auto code = Util::compileGLSLtoSPIRV(glslShader.c_str(), EShLanguage::EShLangCompute);

	auto &device = tester.getDevice();
	auto &physicalDevice = tester.getPhysicalDevice();
	queue = device.getQueue(0, 0);  // TODO: Don't assume this queue can do compute.

	size_t numElements = state.range(0);
	size_t inOffset = 0;
	size_t outOffset = numElements;
	size_t buffersTotalElements = 2 * numElements;
	size_t buffersSize = sizeof(uint32_t) * buffersTotalElements;

	// TODO: vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);
	vk::MemoryAllocateInfo allocateInfo;
	allocateInfo.allocationSize = buffersSize;  // TODO: memoryRequirements.size
	allocateInfo.memoryTypeIndex = 0;           // TODO: memoryRequirements.memoryTypeBits
	deviceMemory = device.allocateMemory(allocateInfo);

	uint32_t *buffers = (uint32_t *)device.mapMemory(deviceMemory, 0, buffersSize);
	memset(buffers, 0, buffersSize);

	for(size_t i = 0; i < numElements; i++)
	{
		buffers[inOffset + i] = (uint32_t)i;
	}

	device.unmapMemory(deviceMemory);
	buffers = nullptr;

	vk::BufferCreateInfo bufferCreateInfo({}, sizeof(uint32_t) * numElements, vk::BufferUsageFlagBits::eStorageBuffer);
	bufferIn = device.createBuffer(bufferCreateInfo);
	device.bindBufferMemory(bufferIn, deviceMemory, sizeof(uint32_t) * inOffset);

	bufferOut = device.createBuffer(bufferCreateInfo);
	device.bindBufferMemory(bufferOut, deviceMemory, sizeof(uint32_t) * outOffset);

	vk::ShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.codeSize = code.size() * sizeof(uint32_t);
	moduleCreateInfo.pCode = (uint32_t *)code.data();
	vk::ShaderModule shaderModule = device.createShaderModule(moduleCreateInfo);

	vk::DescriptorSetLayoutBinding in;
	in.binding = 0;
	in.descriptorCount = 1;
	in.descriptorType = vk::DescriptorType::eStorageBuffer;
	in.stageFlags = vk::ShaderStageFlagBits::eCompute;

	vk::DescriptorSetLayoutBinding out;
	out.binding = 1;
	out.descriptorCount = 1;
	out.descriptorType = vk::DescriptorType::eStorageBuffer;
	out.stageFlags = vk::ShaderStageFlagBits::eCompute;

	std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings = { in, out };
	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	layoutInfo.pBindings = setLayoutBindings.data();
	descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	vk::PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	vk::ComputePipelineCreateInfo computePipelineCreateInfo;
	computePipelineCreateInfo.layout = pipelineLayout;
	computePipelineCreateInfo.stage.stage = vk::ShaderStageFlagBits::eCompute;
	computePipelineCreateInfo.stage.module = shaderModule;
	computePipelineCreateInfo.stage.pName = "main";
	pipeline = device.createComputePipeline({}, computePipelineCreateInfo).value;

	// "A shader module can be destroyed while pipelines created using its shaders are still in use."
	device.destroyShaderModule(shaderModule);

	std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
	poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
	poolSizes[0].descriptorCount = 2;
	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	auto descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);

	vk::DescriptorBufferInfo inBufferInfo;
	inBufferInfo.buffer = bufferIn;
	inBufferInfo.offset = 0;
	inBufferInfo.range = VK_WHOLE_SIZE;

	vk::DescriptorBufferInfo outBufferInfo;
	outBufferInfo.buffer = bufferOut;
	outBufferInfo.offset = 0;
	outBufferInfo.range = VK_WHOLE_SIZE;

	std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {};

	descriptorWrites[0].dstSet = descriptorSets[0];
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &inBufferInfo;

	descriptorWrites[1].dstSet = descriptorSets[0];
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = vk::DescriptorType::eStorageBuffer;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pBufferInfo = &outBufferInfo;

	device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	vk::CommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.queueFamilyIndex = 0;  // TODO: Don't assume queue family 0 can do compute.
	commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPool = device.createCommandPool(commandPoolCreateInfo);

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	auto commandBuffers = device.allocateCommandBuffers(commandBufferAllocateInfo);

	// Record the command buffer
	commandBuffer = commandBuffers[0];

	vk::CommandBufferBeginInfo commandBufferBeginInfo;
	commandBuffer.begin(commandBufferBeginInfo);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, 1, &descriptorSets[0], 0, nullptr);

	commandBuffer.dispatch((uint32_t)(numElements / localSizeX), 1, 1);

	commandBuffer.end();

	// Destroy objects we don't have to hold on to after command buffer recording.
	// "A VkPipelineLayout object must not be destroyed while any command buffer that uses it is in the recording state."
	device.destroyPipelineLayout(pipelineLayout);
}

void BufferToBufferComputeBenchmark::run()
{
	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	queue.submit(submitInfo);
	queue.waitIdle();
}

// Performs an operation `op` on each element.
class ComputeOp : public BufferToBufferComputeBenchmark
{
public:
	ComputeOp(const benchmark::State &state, const char *op, const char *precision)
	    : BufferToBufferComputeBenchmark(state)
	{
		std::stringstream src;
		src << R"(#version 450
			layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
			layout(binding = 0, std430) buffer InBuffer
			{
				float Data[];
			} In;
			layout(binding = 1, std430) buffer OutBuffer
			{
				float Data[];
			} Out;
			void main()
			{
				)"
		    << precision << R"( float x = In.Data[gl_GlobalInvocationID.x];
				Out.Data[gl_GlobalInvocationID.x] = )"
		    << op << R"( (x);
			})";

		initialize(src.str());
	}
};

static void Compute(benchmark::State &state, const char *op, const char *precision = "highp")
{
	ComputeOp benchmark(state, op, precision);

	// Execute once to have the Reactor routine generated.
	benchmark.run();

	for(auto _ : state)
	{
		benchmark.run();
	}
}

BENCHMARK_CAPTURE(Compute, mov, "")->RangeMultiplier(2)->Range(128, 4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();

BENCHMARK_CAPTURE(Compute, sqrt_highp, "sqrt", "highp")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(Compute, sin_highp, "sin", "highp")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(Compute, cos_highp, "cos", "highp")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(Compute, exp_highp, "exp", "highp")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(Compute, log_highp, "log", "highp")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();

BENCHMARK_CAPTURE(Compute, sqrt_mediump, "sqrt", "mediump")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(Compute, sin_mediump, "sin", "mediump")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(Compute, cos_mediump, "cos", "mediump")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(Compute, exp_mediump, "exp", "mediump")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(Compute, log_mediump, "log", "mediump")->Arg(4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();