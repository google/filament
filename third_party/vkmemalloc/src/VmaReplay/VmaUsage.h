#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

//#define VMA_USE_STL_CONTAINERS 1

//#define VMA_HEAVY_ASSERT(expr) assert(expr)

//#define VMA_DEDICATED_ALLOCATION 0

//#define VMA_DEBUG_MARGIN 16
//#define VMA_DEBUG_DETECT_CORRUPTION 1
//#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1

#pragma warning(push, 4)
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4189) // local variable is initialized but not referenced

#include "../vk_mem_alloc.h"

#pragma warning(pop)
