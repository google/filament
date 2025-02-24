/*!*********************************************************************************************************************
\File         MainLinuxNullWS.cpp
\Title        Main Linux NullWS
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Adds the entry point for running the example on a NWS surface platform.
***********************************************************************************************************************/

#include "VulkanHelloAPI.h"

#ifdef USE_PLATFORM_NULLWS
int main(int /*argc*/, char** /*argv*/)
{
	VulkanHelloAPI vulkanExample;
	vulkanExample.initialize();
	vulkanExample.recordCommandBuffer();

	for (uint32_t i = 0; i < 800; ++i) { vulkanExample.drawFrame(); }

	vulkanExample.deinitialize();
}
#endif
