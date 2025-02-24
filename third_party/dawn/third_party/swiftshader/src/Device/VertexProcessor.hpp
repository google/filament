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

#ifndef sw_VertexProcessor_hpp
#define sw_VertexProcessor_hpp

#include "Context.hpp"
#include "Memset.hpp"
#include "RoutineCache.hpp"
#include "Vertex.hpp"
#include "Pipeline/SpirvShader.hpp"

#include <memory>

namespace sw {

struct DrawData;

// Basic direct mapped vertex cache.
struct VertexCache
{
	static constexpr uint32_t SIZE = 64;            // TODO: Variable size?
	static constexpr uint32_t TAG_MASK = SIZE - 1;  // Size must be power of 2.

	void clear();

	Vertex vertex[SIZE];
	uint32_t tag[SIZE];

	// Identifier of the draw call for the cache data. If this cache is
	// used with a different draw call, then the cache should be invalidated
	// before use.
	int drawCall = -1;
};

struct VertexTask
{
	unsigned int vertexCount;
	unsigned int primitiveStart;
	VertexCache vertexCache;
};

using VertexRoutineFunction = FunctionT<void(const vk::Device *device, Vertex *output, unsigned int *batch, VertexTask *vertextask, DrawData *draw)>;

class VertexProcessor
{
public:
	struct States : Memset<States>
	{
		States()
		    : Memset(this, 0)
		{}

		uint32_t computeHash();

		uint64_t shaderID;
		uint32_t pipelineLayoutIdentifier;

		struct Input
		{
			operator bool() const  // Returns true if stream contains data
			{
				return format != VK_FORMAT_UNDEFINED;
			}

			VkFormat format;  // TODO(b/148016460): Could be restricted to VK_FORMAT_END_RANGE
			unsigned int attribType : BITS(SpirvShader::ATTRIBTYPE_LAST);
		};

		Input input[MAX_INTERFACE_COMPONENTS / 4];
		bool robustBufferAccess : 1;
		bool isPoint : 1;
		bool depthClipEnable : 1;
		bool depthClipNegativeOneToOne : 1;
	};

	struct State : States
	{
		bool operator==(const State &state) const;

		uint32_t hash;
	};

	using RoutineType = VertexRoutineFunction::RoutineType;

	VertexProcessor();

	const State update(const vk::GraphicsState &pipelineState, const sw::SpirvShader *vertexShader, const vk::Inputs &inputs);
	RoutineType routine(const State &state, const vk::PipelineLayout *pipelineLayout,
	                    const SpirvShader *vertexShader, const vk::DescriptorSet::Bindings &descriptorSets);

	void setRoutineCacheSize(int cacheSize);

private:
	using RoutineCacheType = RoutineCache<State, VertexRoutineFunction::CFunctionType>;
	std::unique_ptr<RoutineCacheType> routineCache;
};

}  // namespace sw

namespace std {

template<>
struct hash<sw::VertexProcessor::State>
{
	uint64_t operator()(const sw::VertexProcessor::State &state) const
	{
		return state.hash;
	}
};

}  // namespace std

#endif  // sw_VertexProcessor_hpp
