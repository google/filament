# For Fuchsia, we must not export anything other than loader-related API
# There is also the vk_icdInitializeConnectToServiceCallback entry point
# that is specific to the Fuchsia Vulkan loader implementation.
{
global:
	# Loader-ICD interface functions
	vk_icdGetInstanceProcAddr;
	vk_icdNegotiateLoaderICDInterfaceVersion;
	vk_icdGetPhysicalDeviceProcAddr;
	vk_icdInitializeConnectToServiceCallback;

local:
	*;
};

