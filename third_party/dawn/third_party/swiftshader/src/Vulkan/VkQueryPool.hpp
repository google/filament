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

#ifndef VK_QUERY_POOL_HPP_
#define VK_QUERY_POOL_HPP_

#include "VkObject.hpp"

#include "marl/event.h"
#include "marl/waitgroup.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace vk {

class Query
{
public:
	Query(VkQueryType type);

	enum State
	{
		UNAVAILABLE,
		ACTIVE,
		FINISHED
	};

	struct Data
	{
		State state;    // The current query state.
		int64_t value;  // The current query value.
	};

	// reset() sets the state of the Query to UNAVAILABLE, sets the type to
	// INVALID_TYPE and clears the query value.
	// reset() must not be called while the query is in the ACTIVE state.
	void reset();

	// start() begins a query task which is closed with a call to finish().
	// Query tasks can be nested.
	// start() sets the state to ACTIVE.
	void start();

	// finish() ends a query task begun with a call to start().
	// Once all query tasks are complete the query will transition to the
	// FINISHED state.
	// finish() must only be called when in the ACTIVE state.
	void finish();

	// wait() blocks until the query reaches the FINISHED state.
	void wait();

	// getData() returns the current query state and value.
	Data getData() const;

	// getType() returns the type of query.
	VkQueryType getType() const;

	// set() replaces the current query value with val.
	void set(int64_t val);

	// add() adds val to the current query value.
	void add(int64_t val);

private:
	marl::WaitGroup wg;
	marl::Event finished;
	std::atomic<State> state;
	std::atomic<VkQueryType> type;
	std::atomic<int64_t> value;
};

class QueryPool : public Object<QueryPool, VkQueryPool>
{
public:
	QueryPool(const VkQueryPoolCreateInfo *pCreateInfo, void *mem);
	void destroy(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkQueryPoolCreateInfo *pCreateInfo);

	VkResult getResults(uint32_t firstQuery, uint32_t queryCount, size_t dataSize,
	                    void *pData, VkDeviceSize stride, VkQueryResultFlags flags) const;
	void begin(uint32_t query, VkQueryControlFlags flags);
	void end(uint32_t query);
	void reset(uint32_t firstQuery, uint32_t queryCount);

	void writeTimestamp(uint32_t query);

	inline Query *getQuery(uint32_t query) const { return &pool[query]; }
	inline VkQueryType getType() const { return type; }

private:
	Query *const pool;
	const VkQueryType type;
	const uint32_t count;
};

static inline QueryPool *Cast(VkQueryPool object)
{
	return QueryPool::Cast(object);
}

}  // namespace vk

#endif  // VK_QUERY_POOL_HPP_
