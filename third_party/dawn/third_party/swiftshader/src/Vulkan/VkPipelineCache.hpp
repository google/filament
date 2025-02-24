// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_PIPELINE_CACHE_HPP_
#define VK_PIPELINE_CACHE_HPP_

#include "VkObject.hpp"
#include "VkSpecializationInfo.hpp"
#include "Pipeline/SpirvBinary.hpp"

#include "marl/mutex.h"
#include "marl/tsa.h"

#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace sw {

class ComputeProgram;
class SpirvShader;

}  // namespace sw

namespace vk {

class PipelineLayout;
class RenderPass;

class PipelineCache : public Object<PipelineCache, VkPipelineCache>
{
public:
	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_CACHE; }

	PipelineCache(const VkPipelineCacheCreateInfo *pCreateInfo, void *mem);
	virtual ~PipelineCache();
	void destroy(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkPipelineCacheCreateInfo *pCreateInfo);

	VkResult getData(size_t *pDataSize, void *pData);
	VkResult merge(uint32_t srcCacheCount, const VkPipelineCache *pSrcCaches);

	struct SpirvBinaryKey
	{
		SpirvBinaryKey(const sw::SpirvBinary &spirv,
		               const VkSpecializationInfo *specializationInfo,
		               bool robustBufferAccess,
		               bool optimize);

		bool operator<(const SpirvBinaryKey &other) const;

		const sw::SpirvBinary &getBinary() const { return spirv; }
		const VkSpecializationInfo *getSpecializationInfo() const { return specializationInfo.get(); }
		bool getOptimization() const { return optimize; }

	private:
		const sw::SpirvBinary spirv;
		const vk::SpecializationInfo specializationInfo;
		const bool robustBufferAccess;
		const bool optimize;
	};

	// contains() queries whether the cache contains a shader with the given key.
	inline bool contains(const PipelineCache::SpirvBinaryKey &key);

	// getOrOptimizeSpirv() queries the cache for a shader with the given key.
	// If one is found, it is returned, otherwise create() is called, the
	// returned SPIR-V binary is added to the cache, and it is returned.
	// CreateOnCacheMiss must be a function of the signature:
	//     sw::ShaderBinary()
	template<typename CreateOnCacheMiss, typename CacheHit>
	inline sw::SpirvBinary getOrOptimizeSpirv(const PipelineCache::SpirvBinaryKey &key, CreateOnCacheMiss &&create, CacheHit &&cacheHit);

	struct ComputeProgramKey
	{
		ComputeProgramKey(uint64_t shaderIdentifier, uint32_t pipelineLayoutIdentifier);

		bool operator<(const ComputeProgramKey &other) const;

	private:
		const uint64_t shaderIdentifier;
		const uint32_t pipelineLayoutIdentifier;
	};

	// getOrCreateComputeProgram() queries the cache for a compute program with
	// the given key.
	// If one is found, it is returned, otherwise create() is called, the
	// returned program is added to the cache, and it is returned.
	// Function must be a function of the signature:
	//     std::shared_ptr<sw::ComputeProgram>()
	template<typename Function>
	inline std::shared_ptr<sw::ComputeProgram> getOrCreateComputeProgram(const PipelineCache::ComputeProgramKey &key, Function &&create);

private:
	struct CacheHeader
	{
		uint32_t headerLength;
		uint32_t headerVersion;
		uint32_t vendorID;
		uint32_t deviceID;
		uint8_t pipelineCacheUUID[VK_UUID_SIZE];
	};

	size_t dataSize = 0;
	uint8_t *data = nullptr;

	marl::mutex spirvShadersMutex;
	std::map<SpirvBinaryKey, sw::SpirvBinary> spirvShaders GUARDED_BY(spirvShadersMutex);

	marl::mutex computeProgramsMutex;
	std::map<ComputeProgramKey, std::shared_ptr<sw::ComputeProgram>> computePrograms GUARDED_BY(computeProgramsMutex);
};

static inline PipelineCache *Cast(VkPipelineCache object)
{
	return PipelineCache::Cast(object);
}

template<typename Function>
std::shared_ptr<sw::ComputeProgram> PipelineCache::getOrCreateComputeProgram(const PipelineCache::ComputeProgramKey &key, Function &&create)
{
	marl::lock lock(computeProgramsMutex);

	auto it = computePrograms.find(key);
	if(it != computePrograms.end())
	{
		return it->second;
	}

	auto created = create();
	computePrograms.emplace(key, created);

	return created;
}

inline bool PipelineCache::contains(const PipelineCache::SpirvBinaryKey &key)
{
	marl::lock lock(spirvShadersMutex);

	return spirvShaders.find(key) != spirvShaders.end();
}

template<typename CreateOnCacheMiss, typename CacheHit>
sw::SpirvBinary PipelineCache::getOrOptimizeSpirv(const PipelineCache::SpirvBinaryKey &key, CreateOnCacheMiss &&create, CacheHit &&cacheHit)
{
	marl::lock lock(spirvShadersMutex);

	auto it = spirvShaders.find(key);
	if(it != spirvShaders.end())
	{
		cacheHit();
		return it->second;
	}

	sw::SpirvBinary outShader = create();
	spirvShaders.emplace(key, outShader);
	return outShader;
}

}  // namespace vk

#endif  // VK_PIPELINE_CACHE_HPP_
