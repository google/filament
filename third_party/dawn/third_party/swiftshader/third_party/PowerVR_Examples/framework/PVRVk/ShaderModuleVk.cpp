/*!
\brief Function implementations for the ShaderModule class.
\file PVRVk/ShaderModuleVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/ShaderModuleVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
ShaderModule_::ShaderModule_(make_shared_enabler, const DeviceWeakPtr& device, const ShaderModuleCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		assert(false &&
			"loadShader: Generated shader passed to loadShader. "
			"Deleting reference to avoid leaking a preexisting shader object.");
		getDevice()->getVkBindings().vkDestroyShaderModule(getDevice()->getVkHandle(), getVkHandle(), NULL);
	}

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_SHADER_MODULE_CREATE_INFO);
	shaderModuleCreateInfo.codeSize = _createInfo.getCodeSize();
	shaderModuleCreateInfo.pCode = _createInfo.getShaderSources().data();
	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateShaderModule(getDevice()->getVkHandle(), &shaderModuleCreateInfo, nullptr, &_vkHandle), "Failed to create ShaderModule");
}

ShaderModule_::~ShaderModule_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_device.expired())
		{
			getDevice()->getVkBindings().vkDestroyShaderModule(getDevice()->getVkHandle(), getVkHandle(), nullptr);
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
