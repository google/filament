/*!
\brief Function implementations for the Command Pool.
\file PVRVk/CommandPoolVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/QueryPoolVk.h"
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
QueryPool_::QueryPool_(make_shared_enabler, const DeviceWeakPtr& device, const QueryPoolCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	VkQueryPoolCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_QUERY_POOL_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkQueryPoolCreateFlags>(_createInfo.getFlags());
	vkCreateInfo.queryType = static_cast<VkQueryType>(_createInfo.getQueryType());
	vkCreateInfo.queryCount = _createInfo.getNumQueries();
	vkCreateInfo.pipelineStatistics = static_cast<VkQueryPipelineStatisticFlags>(_createInfo.getPipelineStatisticFlags());
	vkThrowIfFailed(static_cast<Result>(getDevice()->getVkBindings().vkCreateQueryPool(getDevice()->getVkHandle(), &vkCreateInfo, nullptr, &_vkHandle)), "Failed to create QueryPool");
}
//!\endcond

bool QueryPool_::getResults(uint32_t queryIndex, size_t dataSize, void* data, QueryResultFlags flags) { return getResults(queryIndex, 1, dataSize, data, 0, flags); }

bool QueryPool_::getResults(uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* data, VkDeviceSize stride, QueryResultFlags flags)
{
	return (getDevice()->getVkBindings().vkGetQueryPoolResults(
				getDevice()->getVkHandle(), getVkHandle(), firstQuery, queryCount, dataSize, data, stride, static_cast<VkQueryResultFlags>(flags)) == Result::e_SUCCESS);
}
} // namespace impl
} // namespace pvrvk
