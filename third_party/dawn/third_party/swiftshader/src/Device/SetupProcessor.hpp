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

#ifndef sw_SetupProcessor_hpp
#define sw_SetupProcessor_hpp

#include "Context.hpp"
#include "Memset.hpp"
#include "RoutineCache.hpp"
#include "System/Types.hpp"
#include <Pipeline/SpirvShader.hpp>

#include <memory>

namespace sw {

struct Primitive;
struct Triangle;
struct Polygon;
struct DrawData;

using SetupFunction = FunctionT<int(const vk::Device *device, Primitive *primitive, const Triangle *triangle, const Polygon *polygon, const DrawData *draw)>;

class SetupProcessor
{
public:
	struct States : Memset<States>
	{
		States()
		    : Memset(this, 0)
		{}

		uint32_t computeHash();

		bool isDrawPoint : 1;
		bool isDrawLine : 1;
		bool isDrawTriangle : 1;
		bool fixedPointDepthBuffer : 1;
		bool applyConstantDepthBias : 1;
		bool applySlopeDepthBias : 1;
		bool applyDepthBiasClamp : 1;
		bool interpolateZ : 1;
		bool interpolateW : 1;
		VkFrontFace frontFace : BITS(VK_FRONT_FACE_MAX_ENUM);
		VkCullModeFlags cullMode : BITS(VK_CULL_MODE_FLAG_BITS_MAX_ENUM);
		unsigned int multiSampleCount : 3;  // 1, 2 or 4
		bool enableMultiSampling : 1;
		unsigned int numClipDistances : 4;  // [0 - 8]
		unsigned int numCullDistances : 4;  // [0 - 8]

		SpirvShader::InterfaceComponent gradient[MAX_INTERFACE_COMPONENTS];
	};

	struct State : States
	{
		bool operator==(const State &states) const;

		uint32_t hash;
	};

	using RoutineType = SetupFunction::RoutineType;

	SetupProcessor();

	State update(const vk::GraphicsState &pipelineState, const sw::SpirvShader *fragmentShader, const sw::SpirvShader *vertexShader, const vk::Attachments &attachments) const;
	RoutineType routine(const State &state);

	void setRoutineCacheSize(int cacheSize);

private:
	using RoutineCacheType = RoutineCache<State, SetupFunction::CFunctionType>;
	std::unique_ptr<RoutineCacheType> routineCache;
};

}  // namespace sw

namespace std {

template<>
struct hash<sw::SetupProcessor::State>
{
	uint64_t operator()(const sw::SetupProcessor::State &state) const
	{
		return state.hash;
	}
};

}  // namespace std

#endif  // sw_SetupProcessor_hpp
