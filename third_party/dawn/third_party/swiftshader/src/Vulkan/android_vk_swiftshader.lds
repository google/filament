# For Android, we must not export anything other than loader-related API
{
global:
	# Loader-ICD interface functions
	vk_icdGetInstanceProcAddr;
	vk_icdNegotiateLoaderICDInterfaceVersion;
	vk_icdGetPhysicalDeviceProcAddr;

	# Android HAL module info object
	HMI;

local:
	*;
};

