/*!
\brief Function definitions for the DescriptorSet class.
\file PVRVk/DescriptorSetVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/SamplerVk.h"
#include "PVRVk/BufferVk.h"

#ifdef DEBUG
#include <algorithm>
#endif

namespace pvrvk {
namespace impl {
pvrvk::DescriptorSet DescriptorPool_::allocateDescriptorSet(const DescriptorSetLayout& layout)
{
	DescriptorPool descriptorPool = shared_from_this();
	return DescriptorSet_::constructShared(layout, descriptorPool);
}

//!\cond NO_DOXYGEN
DescriptorPool_::DescriptorPool_(make_shared_enabler, const DeviceWeakPtr& device, const DescriptorPoolCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils()
{
	_createInfo = createInfo;
	VkDescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.sType = static_cast<VkStructureType>(StructureType::e_DESCRIPTOR_POOL_CREATE_INFO);
	descPoolInfo.pNext = NULL;
	descPoolInfo.maxSets = _createInfo.getMaxDescriptorSets();
	descPoolInfo.flags = static_cast<VkDescriptorPoolCreateFlags>(DescriptorPoolCreateFlags::e_FREE_DESCRIPTOR_SET_BIT);
	VkDescriptorPoolSize poolSizes[static_cast<uint32_t>(DescriptorType::e_RANGE_SIZE)];
	uint32_t poolIndex = 0;
	for (uint32_t i = 0; i < static_cast<uint32_t>(DescriptorType::e_RANGE_SIZE); ++i)
	{
		uint32_t count = _createInfo.getNumDescriptorTypes(DescriptorType(i));
		if (count)
		{
			poolSizes[poolIndex].type = static_cast<VkDescriptorType>(i);
			poolSizes[poolIndex].descriptorCount = count;
			++poolIndex;
		}
	} // next type
	descPoolInfo.poolSizeCount = poolIndex;
	descPoolInfo.pPoolSizes = poolSizes;

	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateDescriptorPool(getDevice()->getVkHandle(), &descPoolInfo, nullptr, &_vkHandle), "Create Descriptor Pool failed");
}

DescriptorPool_::~DescriptorPool_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyDescriptorPool(getDevice()->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}

#ifdef DEBUG
static inline bool bindingIdPairComparison(const std::pair<uint32_t, uint32_t>& a, const std::pair<uint32_t, uint32_t>& b) { return a.first < b.first; }
#endif

DescriptorSetLayout_::~DescriptorSetLayout_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyDescriptorSetLayout(getDevice()->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
	clearCreateInfo();
}

DescriptorSetLayout_::DescriptorSetLayout_(make_shared_enabler, const DeviceWeakPtr& device, const DescriptorSetLayoutCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils()
{
	_createInfo = createInfo;
	VkDescriptorSetLayoutCreateInfo vkLayoutCreateInfo = {};
	ArrayOrVector<VkDescriptorSetLayoutBinding, 4> vkBindings(getCreateInfo().getNumBindings());
	const DescriptorSetLayoutCreateInfo::DescriptorSetLayoutBinding* bindings = createInfo.getAllBindings();
	for (uint32_t i = 0; i < createInfo.getNumBindings(); ++i)
	{
		vkBindings[i].descriptorType = static_cast<VkDescriptorType>(bindings[i].descriptorType);
		vkBindings[i].binding = bindings[i].binding;
		vkBindings[i].descriptorCount = bindings[i].descriptorCount;
		vkBindings[i].stageFlags = static_cast<VkShaderStageFlags>(bindings[i].stageFlags);
		vkBindings[i].pImmutableSamplers = nullptr;
		if (bindings[i].immutableSampler) { vkBindings[i].pImmutableSamplers = &bindings[i].immutableSampler->getVkHandle(); }
	}
	vkLayoutCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
	vkLayoutCreateInfo.bindingCount = static_cast<uint32_t>(getCreateInfo().getNumBindings());
	vkLayoutCreateInfo.pBindings = vkBindings.get();
	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateDescriptorSetLayout(getDevice()->getVkHandle(), &vkLayoutCreateInfo, nullptr, &_vkHandle), "Create Descriptor Set Layout failed");
}
//!\endcond
} // namespace impl
} // namespace pvrvk
