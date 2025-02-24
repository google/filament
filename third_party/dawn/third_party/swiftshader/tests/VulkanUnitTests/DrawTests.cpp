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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class DrawTest : public testing::Test
{
};

// Test that a vertex shader with no gl_Position works.
// This was fixed in swiftshader-cl/51808
TEST_F(DrawTest, VertexShaderNoPositionOutput)
{
	DrawTester tester;
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
				// Remove gl_Position on purpose for the test
				//gl_Position = vec4(inPos.xyz, 1.0);
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

	tester.initialize();
	tester.renderFrame();
}
