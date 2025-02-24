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

#include "VkQueryPool.hpp"

#include <chrono>
#include <cstring>
#include <new>

namespace vk {

Query::Query(VkQueryType type)
    : finished(marl::Event::Mode::Manual)
    , state(UNAVAILABLE)
    , type(type)
    , value(0)
{}

void Query::reset()
{
	finished.clear();
	auto prevState = state.exchange(UNAVAILABLE);
	ASSERT(prevState != ACTIVE);
	value = 0;
}

void Query::start()
{
	auto prevState = state.exchange(ACTIVE);
	ASSERT(prevState != FINISHED);  // Must be reset first
	wg.add();
}

void Query::finish()
{
	if(wg.done())
	{
		auto prevState = state.exchange(FINISHED);
		ASSERT(prevState == ACTIVE);
		finished.signal();
	}
}

Query::Data Query::getData() const
{
	Data out;
	out.state = state;
	out.value = value;
	return out;
}

VkQueryType Query::getType() const
{
	return type;
}

void Query::wait()
{
	finished.wait();
}

void Query::set(int64_t v)
{
	value = v;
}

void Query::add(int64_t v)
{
	value += v;
}

QueryPool::QueryPool(const VkQueryPoolCreateInfo *pCreateInfo, void *mem)
    : pool(reinterpret_cast<Query *>(mem))
    , type(pCreateInfo->queryType)
    , count(pCreateInfo->queryCount)
{
	// According to the Vulkan 1.2 spec, section 30. Features:
	// "pipelineStatisticsQuery specifies whether the pipeline statistics
	//  queries are supported. If this feature is not enabled, queries of
	//  type VK_QUERY_TYPE_PIPELINE_STATISTICS cannot be created, and
	//  none of the VkQueryPipelineStatisticFlagBits bits can be set in the
	//  pipelineStatistics member of the VkQueryPoolCreateInfo structure."
	if(type == VK_QUERY_TYPE_PIPELINE_STATISTICS)
	{
		UNSUPPORTED("VkPhysicalDeviceFeatures::pipelineStatisticsQuery");
	}

	// Construct all queries
	for(uint32_t i = 0; i < count; i++)
	{
		new(&pool[i]) Query(type);
	}
}

void QueryPool::destroy(const VkAllocationCallbacks *pAllocator)
{
	for(uint32_t i = 0; i < count; i++)
	{
		pool[i].~Query();
	}

	vk::freeHostMemory(pool, pAllocator);
}

size_t QueryPool::ComputeRequiredAllocationSize(const VkQueryPoolCreateInfo *pCreateInfo)
{
	return sizeof(Query) * pCreateInfo->queryCount;
}

VkResult QueryPool::getResults(uint32_t firstQuery, uint32_t queryCount, size_t dataSize,
                               void *pData, VkDeviceSize stride, VkQueryResultFlags flags) const
{
	// dataSize must be large enough to contain the result of each query
	ASSERT(static_cast<size_t>(stride * queryCount) <= dataSize);

	// The sum of firstQuery and queryCount must be less than or equal to the number of queries
	ASSERT((firstQuery + queryCount) <= count);

	VkResult result = VK_SUCCESS;
	uint8_t *data = static_cast<uint8_t *>(pData);
	for(uint32_t i = firstQuery; i < (firstQuery + queryCount); i++, data += stride)
	{
		auto &query = pool[i];

		if(flags & VK_QUERY_RESULT_WAIT_BIT)  // Must wait for query to finish
		{
			query.wait();
		}

		const auto current = query.getData();

		// "If VK_QUERY_RESULT_WAIT_BIT and VK_QUERY_RESULT_PARTIAL_BIT are both not set
		//  then no result values are written to pData for queries that are in the
		//  unavailable state at the time of the call, and vkGetQueryPoolResults returns
		//  VK_NOT_READY. However, availability state is still written to pData for those
		//  queries if VK_QUERY_RESULT_WITH_AVAILABILITY_BIT is set."
		bool writeResult = true;
		if(current.state == Query::ACTIVE || (current.state == Query::UNAVAILABLE && !(flags & VK_QUERY_RESULT_WAIT_BIT)))
		{
			result = VK_NOT_READY;
			writeResult = (flags & VK_QUERY_RESULT_PARTIAL_BIT);  // Allow writing partial results
		}

		if(flags & VK_QUERY_RESULT_64_BIT)
		{
			uint64_t *result64 = reinterpret_cast<uint64_t *>(data);
			if(writeResult)
			{
				result64[0] = current.value;
			}
			if(flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)  // Output query availablity
			{
				result64[1] = current.state;
			}
		}
		else
		{
			uint32_t *result32 = reinterpret_cast<uint32_t *>(data);
			if(writeResult)
			{
				result32[0] = static_cast<uint32_t>(current.value);
			}
			if(flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)  // Output query availablity
			{
				result32[1] = current.state;
			}
		}
	}

	return result;
}

void QueryPool::begin(uint32_t query, VkQueryControlFlags flags)
{
	ASSERT(query < count);

	// Only accept flags with valid bits set.
	if(flags & ~(VK_QUERY_CONTROL_PRECISE_BIT))
	{
		UNSUPPORTED("vkCmdBeginQuery::flags 0x%08X", int(flags));
	}

	pool[query].start();
}

void QueryPool::end(uint32_t query)
{
	ASSERT(query < count);
	pool[query].finish();
}

void QueryPool::reset(uint32_t firstQuery, uint32_t queryCount)
{
	// The sum of firstQuery and queryCount must be less than or equal to the number of queries
	ASSERT((firstQuery + queryCount) <= count);

	for(uint32_t i = firstQuery; i < (firstQuery + queryCount); i++)
	{
		pool[i].reset();
	}
}

void QueryPool::writeTimestamp(uint32_t query)
{
	ASSERT(query < count);
	ASSERT(type == VK_QUERY_TYPE_TIMESTAMP);

	pool[query].start();
	pool[query].set(std::chrono::time_point_cast<std::chrono::nanoseconds>(
	                    std::chrono::steady_clock::now())
	                    .time_since_epoch()
	                    .count());
	pool[query].finish();
}

}  // namespace vk
