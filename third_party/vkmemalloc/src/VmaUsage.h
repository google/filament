#ifndef VMA_USAGE_H_
#define VMA_USAGE_H_

#ifdef _WIN32

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

/*
In every place where you want to use Vulkan Memory Allocator, define appropriate
macros if you want to configure the library and then include its header to
include all public interface declarations. Example:
*/

//#define VMA_USE_STL_CONTAINERS 1

//#define VMA_HEAVY_ASSERT(expr) assert(expr)

//#define VMA_DEDICATED_ALLOCATION 0

//#define VMA_DEBUG_MARGIN 16
//#define VMA_DEBUG_DETECT_CORRUPTION 1
//#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
//#define VMA_RECORDING_ENABLED 0

#pragma warning(push, 4)
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4189) // local variable is initialized but not referenced

#include "vk_mem_alloc.h"

#pragma warning(pop)

#else // #ifdef _WIN32

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#endif // #ifdef _WIN32

#endif
