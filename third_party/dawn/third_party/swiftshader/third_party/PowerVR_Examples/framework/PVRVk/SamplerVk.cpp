/*!
\brief Function definitions for the Sampler class.
\file PVRVk/SamplerVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/SamplerVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
Sampler_::Sampler_(make_shared_enabler, const DeviceWeakPtr& device, const pvrvk::SamplerCreateInfo& samplerDesc) : PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = static_cast<VkStructureType>(StructureType::e_SAMPLER_CREATE_INFO);
	samplerInfo.addressModeU = static_cast<VkSamplerAddressMode>(samplerDesc.wrapModeU);
	samplerInfo.addressModeV = static_cast<VkSamplerAddressMode>(samplerDesc.wrapModeV);
	samplerInfo.addressModeW = static_cast<VkSamplerAddressMode>(samplerDesc.wrapModeW);
	samplerInfo.borderColor = static_cast<VkBorderColor>(samplerDesc.borderColor);
	samplerInfo.compareEnable = samplerDesc.compareOpEnable;
	samplerInfo.compareOp = static_cast<VkCompareOp>(samplerDesc.compareOp);
	samplerInfo.magFilter = static_cast<VkFilter>(samplerDesc.magFilter);
	samplerInfo.minFilter = static_cast<VkFilter>(samplerDesc.minFilter);
	samplerInfo.maxAnisotropy = samplerDesc.anisotropyMaximum;
	samplerInfo.anisotropyEnable = samplerDesc.enableAnisotropy;
	samplerInfo.maxLod = samplerDesc.lodMaximum;
	samplerInfo.minLod = samplerDesc.lodMinimum;
	samplerInfo.mipLodBias = samplerDesc.lodBias;
	samplerInfo.mipmapMode = static_cast<VkSamplerMipmapMode>(samplerDesc.mipMapMode);
	samplerInfo.unnormalizedCoordinates = samplerDesc.unnormalizedCoordinates;
	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateSampler(getDevice()->getVkHandle(), &samplerInfo, nullptr, &_vkHandle), "Sampler Creation failed");
}

Sampler_::~Sampler_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroySampler(getDevice()->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			reportDestroyedAfterDevice();
		}
	}
}
//!\endcond
} // namespace impl
} // namespace pvrvk
