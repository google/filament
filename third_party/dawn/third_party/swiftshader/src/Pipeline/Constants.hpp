// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_Constants_hpp
#define sw_Constants_hpp

#include "System/Math.hpp"
#include "System/Types.hpp"
#include "Vulkan/VkConfig.hpp"

namespace sw {

// VK_SAMPLE_COUNT_4_BIT
// https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html#primsrast-multisampling
static constexpr float VkSampleLocations4[][2] = {
	{ 0.375, 0.125 },
	{ 0.875, 0.375 },
	{ 0.125, 0.625 },
	{ 0.625, 0.875 },
};

// Vulkan spec sample positions are relative to 0,0 in top left corner, with Y+ going down.
// Convert to our space, with 0,0 in center, and Y+ going up.
static constexpr float SampleLocationsX[4] = {
	VkSampleLocations4[0][0] - 0.5f,
	VkSampleLocations4[1][0] - 0.5f,
	VkSampleLocations4[2][0] - 0.5f,
	VkSampleLocations4[3][0] - 0.5f,
};

static constexpr float SampleLocationsY[4] = {
	VkSampleLocations4[0][1] - 0.5f,
	VkSampleLocations4[1][1] - 0.5f,
	VkSampleLocations4[2][1] - 0.5f,
	VkSampleLocations4[3][1] - 0.5f,
};

// Compute the yMin and yMax multisample offsets so that they are just
// large enough (+/- max range - epsilon) to include sample points
static constexpr int yMinMultiSampleOffset = sw::toFixedPoint(1, vk::SUBPIXEL_PRECISION_BITS) - sw::toFixedPoint(sw::max(SampleLocationsY[0], SampleLocationsY[1], SampleLocationsY[2], SampleLocationsY[3]), vk::SUBPIXEL_PRECISION_BITS) - 1;
static constexpr int yMaxMultiSampleOffset = sw::toFixedPoint(1, vk::SUBPIXEL_PRECISION_BITS) + sw::toFixedPoint(sw::max(SampleLocationsY[0], SampleLocationsY[1], SampleLocationsY[2], SampleLocationsY[3]), vk::SUBPIXEL_PRECISION_BITS) - 1;

struct Constants
{
	Constants();

	unsigned int transposeBit0[16];
	unsigned int transposeBit1[16];
	unsigned int transposeBit2[16];

	ushort4 cWeight[17];
	float4 uvWeight[17];
	float4 uvStart[17];

	unsigned int occlusionCount[16];

	byte8 maskB4Q[16];
	byte8 invMaskB4Q[16];
	word4 maskW4Q[16];
	word4 invMaskW4Q[16];
	dword4 maskD4X[16];
	dword4 invMaskD4X[16];
	qword maskQ0Q[16];
	qword maskQ1Q[16];
	qword maskQ2Q[16];
	qword maskQ3Q[16];
	qword invMaskQ0Q[16];
	qword invMaskQ1Q[16];
	qword invMaskQ2Q[16];
	qword invMaskQ3Q[16];
	dword4 maskX0X[16];
	dword4 maskX1X[16];
	dword4 maskX2X[16];
	dword4 maskX3X[16];
	dword4 invMaskX0X[16];
	dword4 invMaskX1X[16];
	dword4 invMaskX2X[16];
	dword4 invMaskX3X[16];
	dword2 maskD01Q[16];
	dword2 maskD23Q[16];
	dword2 invMaskD01Q[16];
	dword2 invMaskD23Q[16];
	qword2 maskQ01X[16];
	qword2 maskQ23X[16];
	qword2 invMaskQ01X[16];
	qword2 invMaskQ23X[16];
	word4 maskW01Q[4];
	dword4 maskD01X[4];
	word4 mask565Q[8];
	dword2 mask10Q[16];       // 4 bit writemask -> A2B10G10R10 bit patterns, replicated 2x
	word4 mask5551Q[16];      // 4 bit writemask -> A1R5G5B5 bit patterns, replicated 4x
	word4 maskr5g5b5a1Q[16];  // 4 bit writemask -> R5G5B5A1 bit patterns, replicated 4x
	word4 maskb5g5r5a1Q[16];  // 4 bit writemask -> B5G5R5A1 bit patterns, replicated 4x
	word4 mask4rgbaQ[16];     // 4 bit writemask -> R4G4B4A4 bit patterns, replicated 4x
	word4 mask4argbQ[16];     // 4 bit writemask -> A4R4G4B4 bit patterns, replicated 4x
	dword4 mask11X[8];        // 3 bit writemask -> B10G11R11 bit patterns, replicated 4x

	unsigned short sRGBtoLinearFF_FF00[256];

	// Centroid parameters
	float4 sampleX[4][16];
	float4 sampleY[4][16];
	float4 weight[16];

	// Fragment offsets
	int Xf[4];
	int Yf[4];

	const float SampleLocationsX[4] = {
		sw::SampleLocationsX[0],
		sw::SampleLocationsX[1],
		sw::SampleLocationsX[2],
		sw::SampleLocationsX[3],
	};

	const float SampleLocationsY[4] = {
		sw::SampleLocationsY[0],
		sw::SampleLocationsY[1],
		sw::SampleLocationsY[2],
		sw::SampleLocationsY[3],
	};

	float half2float[65536];
};

}  // namespace sw

#endif  // sw_Constants_hpp
