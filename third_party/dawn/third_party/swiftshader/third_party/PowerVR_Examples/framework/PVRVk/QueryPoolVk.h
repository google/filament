/*!
\brief The PVRVk QueryPool, a pool that can create queries.
\file PVRVk/QueryPoolVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
/// <summary>QueryPool creation descriptor.</summary>
struct QueryPoolCreateInfo
{
public:
	/// <summary>Constructor</summary>
	/// <param name="queryType">The type of queries managed by the pool</param>
	/// <param name="queryCount">The number of queries managed by the pool</param>
	/// <param name="pipelineStatistics">Specifies which counters will be returned in queries on the pool</param>
	/// <param name="flags">Flags to use for creating the query pool</param>
	QueryPoolCreateInfo(QueryType queryType, uint32_t queryCount, QueryPipelineStatisticFlags pipelineStatistics = QueryPipelineStatisticFlags::e_NONE,
		QueryPoolCreateFlags flags = QueryPoolCreateFlags::e_NONE)
		: _flags(flags), _queryType(queryType), _queryCount(queryCount), _pipelineStatistics(pipelineStatistics)
	{}

	/// <summary>Get the query pool creation flags</summary>
	/// <returns>The set of query pool creation flags</returns>
	inline QueryPoolCreateFlags getFlags() const { return _flags; }
	/// <summary>Set the query pool creation flags</summary>
	/// <param name="flags">The query pool creation flags</param>
	inline void setFlags(QueryPoolCreateFlags flags) { this->_flags = flags; }
	/// <summary>Get the set of counters which will be returned in queries on the pool</summary>
	/// <returns>The set of counters which will be returned in queries on the pool</returns>
	inline QueryPipelineStatisticFlags getPipelineStatisticFlags() const { return _pipelineStatistics; }
	/// <summary>Set the set counters will be returned in queries on the pool</summary>
	/// <param name="pipelineStatistics">The set of counters which will be returned in queries on the pool</param>
	inline void setPipelineStatisticFlags(QueryPipelineStatisticFlags pipelineStatistics) { this->_pipelineStatistics = pipelineStatistics; }
	/// <summary>Get the type of queries managed by this query pool</summary>
	/// <returns>The type of queries managed by this query pool</returns>
	inline QueryType getQueryType() const { return _queryType; }
	/// <summary>Set the type of queries this query pool can manage</summary>
	/// <param name="queryType">The type of queries the query pool can manage</param>
	inline void setQueryType(QueryType queryType) { this->_queryType = queryType; }
	/// <summary>Get the number of queries managed by the pool</summary>
	/// <returns>The number of queries managed by the pool</returns>
	inline uint32_t getNumQueries() const { return _queryCount; }
	/// <summary>Set the number queries managed by the pool</summary>
	/// <param name="queryCount">The number of queries to be managed by the pool</param>
	inline void setNumQueries(uint32_t queryCount) { this->_queryCount = queryCount; }

private:
	/// <summary>Flags to use for creating the query pool</summary>
	QueryPoolCreateFlags _flags;
	/// <summary>The type of queries managed by the pool</summary>
	QueryType _queryType;
	/// <summary>The number of queries managed by the pool</summary>
	uint32_t _queryCount;
	/// <summary>Specifies which counters will be returned in queries on the pool</summary>
	QueryPipelineStatisticFlags _pipelineStatistics;
};

namespace impl {
/// <summary>Vulkan implementation of the Query Pool class.
/// Destroying the query pool will also destroy the queries allocated from this pool</summary>
class QueryPool_ : public PVRVkDeviceObjectBase<VkQueryPool, ObjectType::e_QUERY_POOL>, public DeviceObjectDebugUtils<QueryPool_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class QueryPool_;
	};

	static QueryPool constructShared(const DeviceWeakPtr& device, const QueryPoolCreateInfo& createInfo)
	{
		return std::make_shared<QueryPool_>(make_shared_enabler{}, device, createInfo);
	}

	QueryPoolCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(QueryPool_)
	QueryPool_(make_shared_enabler, const DeviceWeakPtr& device, const QueryPoolCreateInfo& createInfo);

	~QueryPool_()
	{
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			if (!_device.expired())
			{
				getDevice()->getVkBindings().vkDestroyQueryPool(getDevice()->getVkHandle(), getVkHandle(), nullptr);
				_vkHandle = VK_NULL_HANDLE;
			}
			else
			{
				reportDestroyedAfterDevice();
			}
		}
	}
	//!\endcond

	/// <summary>Retrieves the status and results for a particular query</summary>
	/// <param name="queryIndex">The initial query index</param>
	/// <param name="dataSize">The size of bytes of the buffer pointed to by data</param>
	/// <param name="data">A pointer to a user allocated buffer where the results will be written</param>
	/// <param name="flags">Specifies how and when results are returned</param>
	/// <returns>True if the results or status' for the set of queries were successfully retrieved</returns>
	bool getResults(uint32_t queryIndex, size_t dataSize, void* data, QueryResultFlags flags);

	/// <summary>Retrieves the status and results for a set of queries</summary>
	/// <param name="firstQuery">The initial query index</param>
	/// <param name="queryCount">The number of queries</param>
	/// <param name="dataSize">The size of bytes of the buffer pointed to by data</param>
	/// <param name="data">A pointer to a user allocated buffer where the results will be written</param>
	/// <param name="stride">The stide in bytes between results for individual queries within data</param>
	/// <param name="flags">Specifies how and when results are returned</param>
	/// <returns>True if the results or statuses for the set of queries were successfully retrieved</returns>
	bool getResults(uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* data, VkDeviceSize stride, QueryResultFlags flags);

	/// <summary>Get the query pool creation flags</summary>
	/// <returns>The set of query pool creation flags</returns>
	inline QueryPoolCreateFlags getFlags() const { return _createInfo.getFlags(); }
	/// <summary>Get the set of counters which will be returned in queries on the pool</summary>
	/// <returns>The set of counters which will be returned in queries on the pool</returns>
	inline QueryPipelineStatisticFlags getQueryPipelineStatisticFlags() const { return _createInfo.getPipelineStatisticFlags(); }
	/// <summary>Get the type of queries managed by this query pool</summary>
	/// <returns>The type of queries managed by this query pool</returns>
	inline QueryType getQueryType() const { return _createInfo.getQueryType(); }
	/// <summary>Get the number of queries managed by the pool</summary>
	/// <returns>The number of queries managed by the pool</returns>
	inline uint32_t getNumQueries() const { return _createInfo.getNumQueries(); }
	/// <summary>Get this query pool's create flags</summary>
	/// <returns>QueryPoolCreateInfo</returns>
	QueryPoolCreateInfo getCreateInfo() const { return _createInfo; }
};
} // namespace impl
} // namespace pvrvk
