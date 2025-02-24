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

#include "Buffer.hpp"
#include "DrawTester.hpp"
#include "benchmark/benchmark.h"

#include <cassert>
#include <vector>

template<typename T>
static void RunBenchmark(benchmark::State &state, T &tester)
{
	tester.initialize();

	if(false) tester.show();  // Enable for visual verification.

	// Warmup
	tester.renderFrame();

	for(auto _ : state)
	{
		tester.renderFrame();
	}
}

static void TriangleSolidColor(benchmark::State &state, Multisample multisample)
{
	DrawTester tester(multisample);

	tester.onCreateVertexBuffers([](DrawTester &tester) {
		struct Vertex
		{
			float position[3];
		};

		Vertex vertexBufferData[] = {
			{ { 1.0f, 1.0f, 0.5f } },
			{ { -1.0f, 1.0f, 0.5f } },
			{ { 0.0f, -1.0f, 0.5f } }
		};

		std::vector<vk::VertexInputAttributeDescription> inputAttributes;
		inputAttributes.push_back(vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)));

		tester.addVertexBuffer(vertexBufferData, sizeof(vertexBufferData), std::move(inputAttributes));
	});

	tester.onCreateVertexShader([](DrawTester &tester) {
		const char *vertexShader = R"(#version 310 es
			layout(location = 0) in vec3 inPos;

			void main()
			{
				gl_Position = vec4(inPos.xyz, 1.0);
			})";

		return tester.createShaderModule(vertexShader, EShLanguage::EShLangVertex);
	});

	tester.onCreateFragmentShader([](DrawTester &tester) {
		const char *fragmentShader = R"(#version 310 es
			precision highp float;

			layout(location = 0) out vec4 outColor;

			void main()
			{
				outColor = vec4(1.0, 1.0, 1.0, 1.0);
			})";

		return tester.createShaderModule(fragmentShader, EShLanguage::EShLangFragment);
	});

	RunBenchmark(state, tester);
}

static void TriangleInterpolateColor(benchmark::State &state, Multisample multisample)
{
	DrawTester tester(multisample);

	tester.onCreateVertexBuffers([](DrawTester &tester) {
		struct Vertex
		{
			float position[3];
			float color[3];
		};

		Vertex vertexBufferData[] = {
			{ { 1.0f, 1.0f, 0.05f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f, 1.0f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
			{ { 0.0f, -1.0f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
		};

		std::vector<vk::VertexInputAttributeDescription> inputAttributes;
		inputAttributes.push_back(vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)));
		inputAttributes.push_back(vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)));

		tester.addVertexBuffer(vertexBufferData, sizeof(vertexBufferData), std::move(inputAttributes));
	});

	tester.onCreateVertexShader([](DrawTester &tester) {
		const char *vertexShader = R"(#version 310 es
			layout(location = 0) in vec3 inPos;
			layout(location = 1) in vec3 inColor;

			layout(location = 0) out vec3 outColor;

			void main()
			{
				outColor = inColor;
				gl_Position = vec4(inPos.xyz, 1.0);
			})";

		return tester.createShaderModule(vertexShader, EShLanguage::EShLangVertex);
	});

	tester.onCreateFragmentShader([](DrawTester &tester) {
		const char *fragmentShader = R"(#version 310 es
			precision highp float;

			layout(location = 0) in vec3 inColor;

			layout(location = 0) out vec4 outColor;

			void main()
			{
				outColor = vec4(inColor, 1.0);
			})";

		return tester.createShaderModule(fragmentShader, EShLanguage::EShLangFragment);
	});

	RunBenchmark(state, tester);
}

static void TriangleSampleTexture(benchmark::State &state, Multisample multisample)
{
	DrawTester tester(multisample);

	tester.onCreateVertexBuffers([](DrawTester &tester) {
		struct Vertex
		{
			float position[3];
			float texCoord[2];
		};

		Vertex vertexBufferData[] = {
			{ { 1.0f, 1.0f, 0.5f }, { 1.0f, 0.0f } },
			{ { -1.0f, 1.0f, 0.5f }, { 0.0f, 1.0f } },
			{ { 0.0f, -1.0f, 0.5f }, { 0.0f, 0.0f } }
		};

		std::vector<vk::VertexInputAttributeDescription> inputAttributes;
		inputAttributes.push_back(vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)));
		inputAttributes.push_back(vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)));

		tester.addVertexBuffer(vertexBufferData, sizeof(vertexBufferData), std::move(inputAttributes));
	});

	tester.onCreateVertexShader([](DrawTester &tester) {
		const char *vertexShader = R"(#version 310 es
			layout(location = 0) in vec3 inPos;
			layout(location = 1) in vec2 inTexCoord;
			layout(location = 0) out vec2 outTexCoord;

			void main()
			{
				gl_Position = vec4(inPos.xyz, 1.0);
				outTexCoord = inTexCoord;
			})";

		return tester.createShaderModule(vertexShader, EShLanguage::EShLangVertex);
	});

	tester.onCreateFragmentShader([](DrawTester &tester) {
		const char *fragmentShader = R"(#version 310 es
			precision highp float;

			layout(location = 0) in vec2 inTexCoord;
			layout(location = 0) out vec4 outColor;
			layout(binding = 0) uniform sampler2D texSampler;

			void main()
			{
				outColor = texture(texSampler, inTexCoord);
			})";

		return tester.createShaderModule(fragmentShader, EShLanguage::EShLangFragment);
	});

	tester.onCreateDescriptorSetLayouts([](DrawTester &tester) -> std::vector<vk::DescriptorSetLayoutBinding> {
		vk::DescriptorSetLayoutBinding samplerLayoutBinding;
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

		return { samplerLayoutBinding };
	});

	tester.onUpdateDescriptorSet([](DrawTester &tester, vk::CommandPool &commandPool, vk::DescriptorSet &descriptorSet) {
		auto &device = tester.getDevice();
		auto &physicalDevice = tester.getPhysicalDevice();
		auto &queue = tester.getQueue();

		auto &texture = tester.addImage(device, physicalDevice, 16, 16, vk::Format::eR8G8B8A8Unorm).obj;

		// Fill texture with colorful checkerboard
		std::array<uint32_t, 3> rgb = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF };
		int colorIndex = 0;
		vk::DeviceSize bufferSize = 16 * 16 * 4;
		Buffer buffer(device, bufferSize, vk::BufferUsageFlagBits::eTransferSrc);
		uint32_t *data = static_cast<uint32_t *>(buffer.mapMemory());

		for(int i = 0; i < 16; i++)
		{
			for(int j = 0; j < 16; j++)
			{
				if(((i ^ j) & 1) == 0)
				{
					data[i + 16 * j] = rgb[colorIndex++ % rgb.size()];
				}
				else
				{
					data[i + 16 * j] = 0;
				}
			}
		}

		buffer.unmapMemory();

		Util::transitionImageLayout(device, commandPool, queue, texture.getImage(), vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		Util::copyBufferToImage(device, commandPool, queue, buffer.getBuffer(), texture.getImage(), 16, 16);
		Util::transitionImageLayout(device, commandPool, queue, texture.getImage(), vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		auto sampler = tester.addSampler(samplerInfo);

		vk::DescriptorImageInfo imageInfo;
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageInfo.imageView = texture.getImageView();
		imageInfo.sampler = sampler.obj;

		std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {};

		descriptorWrites[0].dstSet = descriptorSet;
		descriptorWrites[0].dstBinding = 1;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &imageInfo;

		device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	});

	RunBenchmark(state, tester);
}

BENCHMARK_CAPTURE(TriangleSolidColor, TriangleSolidColor, Multisample::False)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(TriangleInterpolateColor, TriangleInterpolateColor, Multisample::False)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(TriangleSampleTexture, TriangleSampleTexture, Multisample::False)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(TriangleSolidColor, TriangleSolidColor_Multisample, Multisample::True)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(TriangleInterpolateColor, TriangleInterpolateColor_Multisample, Multisample::True)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK_CAPTURE(TriangleSampleTexture, TriangleSampleTexture_Multisample, Multisample::True)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
