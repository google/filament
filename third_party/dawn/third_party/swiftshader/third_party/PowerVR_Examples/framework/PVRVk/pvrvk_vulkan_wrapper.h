// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See pvrvk_vulkan_wrapper_generator.py for modifications

/*
\brief vulkan.h wrapper used by PVRVk.
\file pvrvk_vulkan_wrapper.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

/* Corresponding to Vulkan registry file version #132# */

//!\cond NO_DOXYGEN
// clang-format off
#pragma once
#include "vulkan/vulkan.h"

#if defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
// undef these macros from the xlib files, they are breaking the framework types.
#undef Success
#undef Enum
#undef None
#undef Always
#undef byte
#undef char8
#undef ShaderStageFlags
#undef capability
#endif

#include <algorithm>
#include <vector>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdio>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#define vsnprintf _vsnprintf
#endif

#if defined(__linux__)
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#endif

#define DEFINE_ENUM_BITWISE_OPERATORS(type_) \
inline type_ operator | (type_ lhs, type_ rhs) \
{ \
	return (type_)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); \
} \
inline type_& operator |= (type_& lhs, type_ rhs) \
{ \
	return lhs = (type_)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); \
} \
inline type_ operator & (type_ lhs, type_ rhs) \
{ \
	return (type_)(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); \
} \
inline type_& operator &= (type_& lhs, type_ rhs) \
{ \
	return lhs = (type_)(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); \
} \
inline type_ operator ^ (type_ lhs, type_ rhs) \
{ \
	return (type_)(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)); \
} \
inline type_& operator ^= (type_& lhs, type_ rhs) \
{ \
	return lhs = (type_)(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)); \
} \
inline type_ operator <<(type_ lhs, uint32_t rhs) \
{ \
	return (type_)(static_cast<uint32_t>(lhs) << rhs); \
} \
inline type_& operator <<=(type_& lhs, uint32_t rhs) \
{ \
	return lhs = (type_)(static_cast<uint32_t>(lhs) << rhs); \
} \
inline type_ operator >>(type_ lhs, uint32_t rhs) \
{ \
	return (type_)(static_cast<uint32_t>(lhs) >> rhs); \
} \
inline type_& operator >>=(type_& lhs, uint32_t rhs) \
{ \
	return lhs = (type_)(static_cast<uint32_t>(lhs) >> rhs); \
} \
inline bool operator ==(type_ lhs, uint32_t rhs) \
{ \
	return static_cast<uint32_t>(lhs) == rhs; \
} \
inline bool operator !=(type_ lhs, uint32_t rhs) \
{ \
	return static_cast<uint32_t>(lhs) != rhs; \
} \
inline bool operator ==(uint32_t lhs, type_ rhs) \
{ \
	return lhs == static_cast<uint32_t>(rhs); \
}\
inline bool operator !=(uint32_t lhs, type_ rhs) \
{ \
	return lhs != static_cast<uint32_t>(rhs); \
}\
inline type_ operator ~(type_ lhs) \
{ \
	return (type_)(~static_cast<uint32_t>(lhs)); \
}\
inline type_ operator *(uint32_t lhs, type_ rhs) \
{ \
	return (type_)(lhs * static_cast<uint32_t>(rhs)); \
}\
inline type_ operator *(type_ lhs, uint32_t rhs) \
{ \
	return (type_)(static_cast<uint32_t>(lhs) * rhs); \
}\

#define DEFINE_EMPTY_TO_STRING(type_)\
inline std::string to_string(type_ value) \
{ \
	(void)value; \
	return "reserved"; \
} \

namespace pvrvk {


// PVRVk Structures defined in PVRVk/Types.h. These are excluded from autogeneration so we need to forward declare them here 
struct ClearValue;
struct ClearColorValue;
struct ClearAttachment;
struct AttachmentDescription;
struct SubpassDescription;

// PVRVk Basetypes
typedef VkFlags Flags;
typedef VkBool32 Bool32;
typedef VkDeviceSize DeviceSize;
typedef VkSampleMask SampleMask;
typedef VkDeviceAddress DeviceAddress;


template<typename MyEnum>
inline void append_to_string_flag(MyEnum current_value, std::string& current_string, MyEnum flag_to_test, const char* string_to_add)
{
	if(static_cast<uint32_t>(current_value & flag_to_test) != 0)
	{
		if (!current_string.empty()){
			current_string += "|";
		}
		current_string += string_to_add;
	}
}


// PVRVk Bitmasks (Empty)
enum class QueryPoolCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(QueryPoolCreateFlags)
DEFINE_EMPTY_TO_STRING(QueryPoolCreateFlags)

enum class PipelineLayoutCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineLayoutCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineLayoutCreateFlags)

enum class PipelineCacheCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineCacheCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineCacheCreateFlags)

enum class PipelineDepthStencilStateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineDepthStencilStateCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineDepthStencilStateCreateFlags)

enum class PipelineDynamicStateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineDynamicStateCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineDynamicStateCreateFlags)

enum class PipelineColorBlendStateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineColorBlendStateCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineColorBlendStateCreateFlags)

enum class PipelineMultisampleStateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineMultisampleStateCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineMultisampleStateCreateFlags)

enum class PipelineRasterizationStateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineRasterizationStateCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineRasterizationStateCreateFlags)

enum class PipelineViewportStateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineViewportStateCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineViewportStateCreateFlags)

enum class PipelineTessellationStateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineTessellationStateCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineTessellationStateCreateFlags)

enum class PipelineInputAssemblyStateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineInputAssemblyStateCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineInputAssemblyStateCreateFlags)

enum class PipelineVertexInputStateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineVertexInputStateCreateFlags)
DEFINE_EMPTY_TO_STRING(PipelineVertexInputStateCreateFlags)

enum class BufferViewCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(BufferViewCreateFlags)
DEFINE_EMPTY_TO_STRING(BufferViewCreateFlags)

enum class InstanceCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(InstanceCreateFlags)
DEFINE_EMPTY_TO_STRING(InstanceCreateFlags)

enum class DeviceCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(DeviceCreateFlags)
DEFINE_EMPTY_TO_STRING(DeviceCreateFlags)

enum class EventCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(EventCreateFlags)
DEFINE_EMPTY_TO_STRING(EventCreateFlags)

enum class MemoryMapFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(MemoryMapFlags)
DEFINE_EMPTY_TO_STRING(MemoryMapFlags)

enum class DescriptorPoolResetFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(DescriptorPoolResetFlags)
DEFINE_EMPTY_TO_STRING(DescriptorPoolResetFlags)

enum class DescriptorUpdateTemplateCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(DescriptorUpdateTemplateCreateFlags)
DEFINE_EMPTY_TO_STRING(DescriptorUpdateTemplateCreateFlags)

enum class DisplayModeCreateFlagsKHR: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(DisplayModeCreateFlagsKHR)
DEFINE_EMPTY_TO_STRING(DisplayModeCreateFlagsKHR)

enum class DisplaySurfaceCreateFlagsKHR: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(DisplaySurfaceCreateFlagsKHR)
DEFINE_EMPTY_TO_STRING(DisplaySurfaceCreateFlagsKHR)

#ifdef VK_USE_PLATFORM_ANDROID_KHR
enum class AndroidSurfaceCreateFlagsKHR: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(AndroidSurfaceCreateFlagsKHR)
DEFINE_EMPTY_TO_STRING(AndroidSurfaceCreateFlagsKHR)
#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_VI_NN
enum class ViSurfaceCreateFlagsNN: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(ViSurfaceCreateFlagsNN)
DEFINE_EMPTY_TO_STRING(ViSurfaceCreateFlagsNN)
#endif // VK_USE_PLATFORM_VI_NN

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
enum class WaylandSurfaceCreateFlagsKHR: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(WaylandSurfaceCreateFlagsKHR)
DEFINE_EMPTY_TO_STRING(WaylandSurfaceCreateFlagsKHR)
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
enum class Win32SurfaceCreateFlagsKHR: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(Win32SurfaceCreateFlagsKHR)
DEFINE_EMPTY_TO_STRING(Win32SurfaceCreateFlagsKHR)
#endif // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
enum class XlibSurfaceCreateFlagsKHR: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(XlibSurfaceCreateFlagsKHR)
DEFINE_EMPTY_TO_STRING(XlibSurfaceCreateFlagsKHR)
#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
enum class XcbSurfaceCreateFlagsKHR: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(XcbSurfaceCreateFlagsKHR)
DEFINE_EMPTY_TO_STRING(XcbSurfaceCreateFlagsKHR)
#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_IOS_MVK
enum class IOSSurfaceCreateFlagsMVK: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(IOSSurfaceCreateFlagsMVK)
DEFINE_EMPTY_TO_STRING(IOSSurfaceCreateFlagsMVK)
#endif // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
enum class MacOSSurfaceCreateFlagsMVK: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(MacOSSurfaceCreateFlagsMVK)
DEFINE_EMPTY_TO_STRING(MacOSSurfaceCreateFlagsMVK)
#endif // VK_USE_PLATFORM_MACOS_MVK

#ifdef VK_USE_PLATFORM_METAL_EXT
enum class MetalSurfaceCreateFlagsEXT: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(MetalSurfaceCreateFlagsEXT)
DEFINE_EMPTY_TO_STRING(MetalSurfaceCreateFlagsEXT)
#endif // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_FUCHSIA
enum class ImagePipeSurfaceCreateFlagsFUCHSIA: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(ImagePipeSurfaceCreateFlagsFUCHSIA)
DEFINE_EMPTY_TO_STRING(ImagePipeSurfaceCreateFlagsFUCHSIA)
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_GGP
enum class StreamDescriptorSurfaceCreateFlagsGGP: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(StreamDescriptorSurfaceCreateFlagsGGP)
DEFINE_EMPTY_TO_STRING(StreamDescriptorSurfaceCreateFlagsGGP)
#endif // VK_USE_PLATFORM_GGP

enum class HeadlessSurfaceCreateFlagsEXT: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(HeadlessSurfaceCreateFlagsEXT)
DEFINE_EMPTY_TO_STRING(HeadlessSurfaceCreateFlagsEXT)

enum class CommandPoolTrimFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(CommandPoolTrimFlags)
DEFINE_EMPTY_TO_STRING(CommandPoolTrimFlags)

enum class PipelineViewportSwizzleStateCreateFlagsNV: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineViewportSwizzleStateCreateFlagsNV)
DEFINE_EMPTY_TO_STRING(PipelineViewportSwizzleStateCreateFlagsNV)

enum class PipelineDiscardRectangleStateCreateFlagsEXT: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineDiscardRectangleStateCreateFlagsEXT)
DEFINE_EMPTY_TO_STRING(PipelineDiscardRectangleStateCreateFlagsEXT)

enum class PipelineCoverageToColorStateCreateFlagsNV: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineCoverageToColorStateCreateFlagsNV)
DEFINE_EMPTY_TO_STRING(PipelineCoverageToColorStateCreateFlagsNV)

enum class PipelineCoverageModulationStateCreateFlagsNV: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineCoverageModulationStateCreateFlagsNV)
DEFINE_EMPTY_TO_STRING(PipelineCoverageModulationStateCreateFlagsNV)

enum class PipelineCoverageReductionStateCreateFlagsNV: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineCoverageReductionStateCreateFlagsNV)
DEFINE_EMPTY_TO_STRING(PipelineCoverageReductionStateCreateFlagsNV)

enum class ValidationCacheCreateFlagsEXT: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(ValidationCacheCreateFlagsEXT)
DEFINE_EMPTY_TO_STRING(ValidationCacheCreateFlagsEXT)

enum class DebugUtilsMessengerCreateFlagsEXT: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(DebugUtilsMessengerCreateFlagsEXT)
DEFINE_EMPTY_TO_STRING(DebugUtilsMessengerCreateFlagsEXT)

enum class DebugUtilsMessengerCallbackDataFlagsEXT: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(DebugUtilsMessengerCallbackDataFlagsEXT)
DEFINE_EMPTY_TO_STRING(DebugUtilsMessengerCallbackDataFlagsEXT)

enum class PipelineRasterizationConservativeStateCreateFlagsEXT: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineRasterizationConservativeStateCreateFlagsEXT)
DEFINE_EMPTY_TO_STRING(PipelineRasterizationConservativeStateCreateFlagsEXT)

enum class PipelineRasterizationStateStreamCreateFlagsEXT: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineRasterizationStateStreamCreateFlagsEXT)
DEFINE_EMPTY_TO_STRING(PipelineRasterizationStateStreamCreateFlagsEXT)

enum class PipelineRasterizationDepthClipStateCreateFlagsEXT: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineRasterizationDepthClipStateCreateFlagsEXT)
DEFINE_EMPTY_TO_STRING(PipelineRasterizationDepthClipStateCreateFlagsEXT)


// PVRVk Bitmasks
enum class CullModeFlags
{
	e_NONE = VK_CULL_MODE_NONE,
	e_FRONT_BIT = VK_CULL_MODE_FRONT_BIT,
	e_BACK_BIT = VK_CULL_MODE_BACK_BIT,
	e_FRONT_AND_BACK = VK_CULL_MODE_FRONT_AND_BACK,
	e_ALL_BITS = e_FRONT_BIT|e_BACK_BIT|e_FRONT_AND_BACK,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CullModeFlags)
inline std::string to_string(CullModeFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, CullModeFlags::e_NONE, "VK_CULL_MODE_NONE");
	append_to_string_flag(value, returnString, CullModeFlags::e_FRONT_BIT, "VK_CULL_MODE_FRONT_BIT");
	append_to_string_flag(value, returnString, CullModeFlags::e_BACK_BIT, "VK_CULL_MODE_BACK_BIT");
	append_to_string_flag(value, returnString, CullModeFlags::e_FRONT_AND_BACK, "VK_CULL_MODE_FRONT_AND_BACK");
	return returnString;
}

enum class QueueFlags
{
	e_NONE = 0,
	e_GRAPHICS_BIT = VK_QUEUE_GRAPHICS_BIT,
	e_COMPUTE_BIT = VK_QUEUE_COMPUTE_BIT,
	e_TRANSFER_BIT = VK_QUEUE_TRANSFER_BIT,
	e_SPARSE_BINDING_BIT = VK_QUEUE_SPARSE_BINDING_BIT,
	e_PROTECTED_BIT = VK_QUEUE_PROTECTED_BIT,
	e_ALL_BITS = e_GRAPHICS_BIT|e_COMPUTE_BIT|e_TRANSFER_BIT|e_SPARSE_BINDING_BIT|e_PROTECTED_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(QueueFlags)
inline std::string to_string(QueueFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, QueueFlags::e_GRAPHICS_BIT, "VK_QUEUE_GRAPHICS_BIT");
	append_to_string_flag(value, returnString, QueueFlags::e_COMPUTE_BIT, "VK_QUEUE_COMPUTE_BIT");
	append_to_string_flag(value, returnString, QueueFlags::e_TRANSFER_BIT, "VK_QUEUE_TRANSFER_BIT");
	append_to_string_flag(value, returnString, QueueFlags::e_SPARSE_BINDING_BIT, "VK_QUEUE_SPARSE_BINDING_BIT");
	append_to_string_flag(value, returnString, QueueFlags::e_PROTECTED_BIT, "VK_QUEUE_PROTECTED_BIT");
	return returnString;
}

enum class RenderPassCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(RenderPassCreateFlags)
DEFINE_EMPTY_TO_STRING(RenderPassCreateFlags)

enum class DeviceQueueCreateFlags
{
	e_NONE = 0,
	e_PROTECTED_BIT = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT,
	e_ALL_BITS = e_PROTECTED_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DeviceQueueCreateFlags)
inline std::string to_string(DeviceQueueCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DeviceQueueCreateFlags::e_PROTECTED_BIT, "VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT");
	return returnString;
}

enum class MemoryPropertyFlags
{
	e_NONE = 0,
	e_DEVICE_LOCAL_BIT = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	e_HOST_VISIBLE_BIT = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	e_HOST_COHERENT_BIT = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	e_HOST_CACHED_BIT = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
	e_LAZILY_ALLOCATED_BIT = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
	e_PROTECTED_BIT = VK_MEMORY_PROPERTY_PROTECTED_BIT,
	e_DEVICE_COHERENT_BIT_AMD = VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD,
	e_DEVICE_UNCACHED_BIT_AMD = VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD,
	e_ALL_BITS = e_DEVICE_LOCAL_BIT|e_HOST_VISIBLE_BIT|e_HOST_COHERENT_BIT|e_HOST_CACHED_BIT|e_LAZILY_ALLOCATED_BIT|e_PROTECTED_BIT|e_DEVICE_COHERENT_BIT_AMD|e_DEVICE_UNCACHED_BIT_AMD,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(MemoryPropertyFlags)
inline std::string to_string(MemoryPropertyFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT");
	append_to_string_flag(value, returnString, MemoryPropertyFlags::e_HOST_VISIBLE_BIT, "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT");
	append_to_string_flag(value, returnString, MemoryPropertyFlags::e_HOST_COHERENT_BIT, "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT");
	append_to_string_flag(value, returnString, MemoryPropertyFlags::e_HOST_CACHED_BIT, "VK_MEMORY_PROPERTY_HOST_CACHED_BIT");
	append_to_string_flag(value, returnString, MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT, "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT");
	append_to_string_flag(value, returnString, MemoryPropertyFlags::e_PROTECTED_BIT, "VK_MEMORY_PROPERTY_PROTECTED_BIT");
	append_to_string_flag(value, returnString, MemoryPropertyFlags::e_DEVICE_COHERENT_BIT_AMD, "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD");
	append_to_string_flag(value, returnString, MemoryPropertyFlags::e_DEVICE_UNCACHED_BIT_AMD, "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD");
	return returnString;
}

enum class MemoryHeapFlags
{
	e_NONE = 0,
	e_DEVICE_LOCAL_BIT = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,
	e_MULTI_INSTANCE_BIT = VK_MEMORY_HEAP_MULTI_INSTANCE_BIT,
	e_MULTI_INSTANCE_BIT_KHR = VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR,
	e_ALL_BITS = e_DEVICE_LOCAL_BIT|e_MULTI_INSTANCE_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(MemoryHeapFlags)
inline std::string to_string(MemoryHeapFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, MemoryHeapFlags::e_DEVICE_LOCAL_BIT, "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT");
	append_to_string_flag(value, returnString, MemoryHeapFlags::e_MULTI_INSTANCE_BIT, "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT");
	return returnString;
}

enum class AccessFlags
{
	e_NONE = 0,
	e_INDIRECT_COMMAND_READ_BIT = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
	e_INDEX_READ_BIT = VK_ACCESS_INDEX_READ_BIT,
	e_VERTEX_ATTRIBUTE_READ_BIT = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
	e_UNIFORM_READ_BIT = VK_ACCESS_UNIFORM_READ_BIT,
	e_INPUT_ATTACHMENT_READ_BIT = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
	e_SHADER_READ_BIT = VK_ACCESS_SHADER_READ_BIT,
	e_SHADER_WRITE_BIT = VK_ACCESS_SHADER_WRITE_BIT,
	e_COLOR_ATTACHMENT_READ_BIT = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
	e_COLOR_ATTACHMENT_WRITE_BIT = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	e_DEPTH_STENCIL_ATTACHMENT_READ_BIT = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
	e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	e_TRANSFER_READ_BIT = VK_ACCESS_TRANSFER_READ_BIT,
	e_TRANSFER_WRITE_BIT = VK_ACCESS_TRANSFER_WRITE_BIT,
	e_HOST_READ_BIT = VK_ACCESS_HOST_READ_BIT,
	e_HOST_WRITE_BIT = VK_ACCESS_HOST_WRITE_BIT,
	e_MEMORY_READ_BIT = VK_ACCESS_MEMORY_READ_BIT,
	e_MEMORY_WRITE_BIT = VK_ACCESS_MEMORY_WRITE_BIT,
	e_COMMAND_PROCESS_READ_BIT_NVX = VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX,
	e_COMMAND_PROCESS_WRITE_BIT_NVX = VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX,
	e_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
	e_CONDITIONAL_RENDERING_READ_BIT_EXT = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
	e_ACCELERATION_STRUCTURE_READ_BIT_NV = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV,
	e_ACCELERATION_STRUCTURE_WRITE_BIT_NV = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV,
	e_SHADING_RATE_IMAGE_READ_BIT_NV = VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV,
	e_FRAGMENT_DENSITY_MAP_READ_BIT_EXT = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
	e_TRANSFORM_FEEDBACK_WRITE_BIT_EXT = VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
	e_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
	e_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
	e_ALL_BITS = e_INDIRECT_COMMAND_READ_BIT|e_INDEX_READ_BIT|e_VERTEX_ATTRIBUTE_READ_BIT|e_UNIFORM_READ_BIT|e_INPUT_ATTACHMENT_READ_BIT|e_SHADER_READ_BIT|e_SHADER_WRITE_BIT|e_COLOR_ATTACHMENT_READ_BIT|e_COLOR_ATTACHMENT_WRITE_BIT|e_DEPTH_STENCIL_ATTACHMENT_READ_BIT|e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT|e_TRANSFER_READ_BIT|e_TRANSFER_WRITE_BIT|e_HOST_READ_BIT|e_HOST_WRITE_BIT|e_MEMORY_READ_BIT|e_MEMORY_WRITE_BIT|e_COMMAND_PROCESS_READ_BIT_NVX|e_COMMAND_PROCESS_WRITE_BIT_NVX|e_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT|e_CONDITIONAL_RENDERING_READ_BIT_EXT|e_ACCELERATION_STRUCTURE_READ_BIT_NV|e_ACCELERATION_STRUCTURE_WRITE_BIT_NV|e_SHADING_RATE_IMAGE_READ_BIT_NV|e_FRAGMENT_DENSITY_MAP_READ_BIT_EXT|e_TRANSFORM_FEEDBACK_WRITE_BIT_EXT|e_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT|e_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(AccessFlags)
inline std::string to_string(AccessFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, AccessFlags::e_INDIRECT_COMMAND_READ_BIT, "VK_ACCESS_INDIRECT_COMMAND_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_INDEX_READ_BIT, "VK_ACCESS_INDEX_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_VERTEX_ATTRIBUTE_READ_BIT, "VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_UNIFORM_READ_BIT, "VK_ACCESS_UNIFORM_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_INPUT_ATTACHMENT_READ_BIT, "VK_ACCESS_INPUT_ATTACHMENT_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_SHADER_READ_BIT, "VK_ACCESS_SHADER_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_SHADER_WRITE_BIT, "VK_ACCESS_SHADER_WRITE_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_COLOR_ATTACHMENT_READ_BIT, "VK_ACCESS_COLOR_ATTACHMENT_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, "VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_READ_BIT, "VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, "VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_TRANSFER_READ_BIT, "VK_ACCESS_TRANSFER_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_TRANSFER_WRITE_BIT, "VK_ACCESS_TRANSFER_WRITE_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_HOST_READ_BIT, "VK_ACCESS_HOST_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_HOST_WRITE_BIT, "VK_ACCESS_HOST_WRITE_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_MEMORY_READ_BIT, "VK_ACCESS_MEMORY_READ_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_MEMORY_WRITE_BIT, "VK_ACCESS_MEMORY_WRITE_BIT");
	append_to_string_flag(value, returnString, AccessFlags::e_COMMAND_PROCESS_READ_BIT_NVX, "VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX");
	append_to_string_flag(value, returnString, AccessFlags::e_COMMAND_PROCESS_WRITE_BIT_NVX, "VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX");
	append_to_string_flag(value, returnString, AccessFlags::e_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT, "VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT");
	append_to_string_flag(value, returnString, AccessFlags::e_CONDITIONAL_RENDERING_READ_BIT_EXT, "VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT");
	append_to_string_flag(value, returnString, AccessFlags::e_ACCELERATION_STRUCTURE_READ_BIT_NV, "VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV");
	append_to_string_flag(value, returnString, AccessFlags::e_ACCELERATION_STRUCTURE_WRITE_BIT_NV, "VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV");
	append_to_string_flag(value, returnString, AccessFlags::e_SHADING_RATE_IMAGE_READ_BIT_NV, "VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV");
	append_to_string_flag(value, returnString, AccessFlags::e_FRAGMENT_DENSITY_MAP_READ_BIT_EXT, "VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT");
	append_to_string_flag(value, returnString, AccessFlags::e_TRANSFORM_FEEDBACK_WRITE_BIT_EXT, "VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT");
	append_to_string_flag(value, returnString, AccessFlags::e_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT, "VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT");
	append_to_string_flag(value, returnString, AccessFlags::e_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT, "VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT");
	return returnString;
}

enum class BufferUsageFlags
{
	e_NONE = 0,
	e_TRANSFER_SRC_BIT = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	e_TRANSFER_DST_BIT = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	e_UNIFORM_TEXEL_BUFFER_BIT = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
	e_STORAGE_TEXEL_BUFFER_BIT = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
	e_UNIFORM_BUFFER_BIT = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	e_STORAGE_BUFFER_BIT = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
	e_INDEX_BUFFER_BIT = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	e_VERTEX_BUFFER_BIT = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	e_INDIRECT_BUFFER_BIT = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
	e_CONDITIONAL_RENDERING_BIT_EXT = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT,
	e_RAY_TRACING_BIT_NV = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
	e_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT,
	e_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
	e_SHADER_DEVICE_ADDRESS_BIT = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	e_SHADER_DEVICE_ADDRESS_BIT_EXT = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT,
	e_SHADER_DEVICE_ADDRESS_BIT_KHR = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR,
	e_ALL_BITS = e_TRANSFER_SRC_BIT|e_TRANSFER_DST_BIT|e_UNIFORM_TEXEL_BUFFER_BIT|e_STORAGE_TEXEL_BUFFER_BIT|e_UNIFORM_BUFFER_BIT|e_STORAGE_BUFFER_BIT|e_INDEX_BUFFER_BIT|e_VERTEX_BUFFER_BIT|e_INDIRECT_BUFFER_BIT|e_CONDITIONAL_RENDERING_BIT_EXT|e_RAY_TRACING_BIT_NV|e_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT|e_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT|e_SHADER_DEVICE_ADDRESS_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(BufferUsageFlags)
inline std::string to_string(BufferUsageFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, BufferUsageFlags::e_TRANSFER_SRC_BIT, "VK_BUFFER_USAGE_TRANSFER_SRC_BIT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_TRANSFER_DST_BIT, "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_UNIFORM_TEXEL_BUFFER_BIT, "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_STORAGE_TEXEL_BUFFER_BIT, "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_UNIFORM_BUFFER_BIT, "VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_STORAGE_BUFFER_BIT, "VK_BUFFER_USAGE_STORAGE_BUFFER_BIT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_INDEX_BUFFER_BIT, "VK_BUFFER_USAGE_INDEX_BUFFER_BIT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_VERTEX_BUFFER_BIT, "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_INDIRECT_BUFFER_BIT, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_CONDITIONAL_RENDERING_BIT_EXT, "VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_RAY_TRACING_BIT_NV, "VK_BUFFER_USAGE_RAY_TRACING_BIT_NV");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT, "VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT, "VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT");
	append_to_string_flag(value, returnString, BufferUsageFlags::e_SHADER_DEVICE_ADDRESS_BIT, "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT");
	return returnString;
}

enum class BufferCreateFlags
{
	e_NONE = 0,
	e_SPARSE_BINDING_BIT = VK_BUFFER_CREATE_SPARSE_BINDING_BIT,
	e_SPARSE_RESIDENCY_BIT = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT,
	e_SPARSE_ALIASED_BIT = VK_BUFFER_CREATE_SPARSE_ALIASED_BIT,
	e_PROTECTED_BIT = VK_BUFFER_CREATE_PROTECTED_BIT,
	e_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT,
	e_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT,
	e_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR,
	e_ALL_BITS = e_SPARSE_BINDING_BIT|e_SPARSE_RESIDENCY_BIT|e_SPARSE_ALIASED_BIT|e_PROTECTED_BIT|e_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(BufferCreateFlags)
inline std::string to_string(BufferCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, BufferCreateFlags::e_SPARSE_BINDING_BIT, "VK_BUFFER_CREATE_SPARSE_BINDING_BIT");
	append_to_string_flag(value, returnString, BufferCreateFlags::e_SPARSE_RESIDENCY_BIT, "VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT");
	append_to_string_flag(value, returnString, BufferCreateFlags::e_SPARSE_ALIASED_BIT, "VK_BUFFER_CREATE_SPARSE_ALIASED_BIT");
	append_to_string_flag(value, returnString, BufferCreateFlags::e_PROTECTED_BIT, "VK_BUFFER_CREATE_PROTECTED_BIT");
	append_to_string_flag(value, returnString, BufferCreateFlags::e_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT, "VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT");
	return returnString;
}

enum class ShaderStageFlags
{
	e_NONE = 0,
	e_VERTEX_BIT = VK_SHADER_STAGE_VERTEX_BIT,
	e_TESSELLATION_CONTROL_BIT = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
	e_TESSELLATION_EVALUATION_BIT = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
	e_GEOMETRY_BIT = VK_SHADER_STAGE_GEOMETRY_BIT,
	e_FRAGMENT_BIT = VK_SHADER_STAGE_FRAGMENT_BIT,
	e_ALL_GRAPHICS = VK_SHADER_STAGE_ALL_GRAPHICS,
	e_COMPUTE_BIT = VK_SHADER_STAGE_COMPUTE_BIT,
	e_TASK_BIT_NV = VK_SHADER_STAGE_TASK_BIT_NV,
	e_MESH_BIT_NV = VK_SHADER_STAGE_MESH_BIT_NV,
	e_RAYGEN_BIT_NV = VK_SHADER_STAGE_RAYGEN_BIT_NV,
	e_ANY_HIT_BIT_NV = VK_SHADER_STAGE_ANY_HIT_BIT_NV,
	e_CLOSEST_HIT_BIT_NV = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
	e_MISS_BIT_NV = VK_SHADER_STAGE_MISS_BIT_NV,
	e_INTERSECTION_BIT_NV = VK_SHADER_STAGE_INTERSECTION_BIT_NV,
	e_CALLABLE_BIT_NV = VK_SHADER_STAGE_CALLABLE_BIT_NV,
	e_ALL = VK_SHADER_STAGE_ALL,
	e_ALL_BITS = e_VERTEX_BIT|e_TESSELLATION_CONTROL_BIT|e_TESSELLATION_EVALUATION_BIT|e_GEOMETRY_BIT|e_FRAGMENT_BIT|e_ALL_GRAPHICS|e_COMPUTE_BIT|e_TASK_BIT_NV|e_MESH_BIT_NV|e_RAYGEN_BIT_NV|e_ANY_HIT_BIT_NV|e_CLOSEST_HIT_BIT_NV|e_MISS_BIT_NV|e_INTERSECTION_BIT_NV|e_CALLABLE_BIT_NV|e_ALL,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ShaderStageFlags)
inline std::string to_string(ShaderStageFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ShaderStageFlags::e_VERTEX_BIT, "VK_SHADER_STAGE_VERTEX_BIT");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_TESSELLATION_CONTROL_BIT, "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_TESSELLATION_EVALUATION_BIT, "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_GEOMETRY_BIT, "VK_SHADER_STAGE_GEOMETRY_BIT");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_FRAGMENT_BIT, "VK_SHADER_STAGE_FRAGMENT_BIT");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_ALL_GRAPHICS, "VK_SHADER_STAGE_ALL_GRAPHICS");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_COMPUTE_BIT, "VK_SHADER_STAGE_COMPUTE_BIT");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_TASK_BIT_NV, "VK_SHADER_STAGE_TASK_BIT_NV");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_MESH_BIT_NV, "VK_SHADER_STAGE_MESH_BIT_NV");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_RAYGEN_BIT_NV, "VK_SHADER_STAGE_RAYGEN_BIT_NV");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_ANY_HIT_BIT_NV, "VK_SHADER_STAGE_ANY_HIT_BIT_NV");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_CLOSEST_HIT_BIT_NV, "VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_MISS_BIT_NV, "VK_SHADER_STAGE_MISS_BIT_NV");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_INTERSECTION_BIT_NV, "VK_SHADER_STAGE_INTERSECTION_BIT_NV");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_CALLABLE_BIT_NV, "VK_SHADER_STAGE_CALLABLE_BIT_NV");
	append_to_string_flag(value, returnString, ShaderStageFlags::e_ALL, "VK_SHADER_STAGE_ALL");
	return returnString;
}

enum class ImageUsageFlags
{
	e_NONE = 0,
	e_TRANSFER_SRC_BIT = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	e_TRANSFER_DST_BIT = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	e_SAMPLED_BIT = VK_IMAGE_USAGE_SAMPLED_BIT,
	e_STORAGE_BIT = VK_IMAGE_USAGE_STORAGE_BIT,
	e_COLOR_ATTACHMENT_BIT = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	e_DEPTH_STENCIL_ATTACHMENT_BIT = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	e_TRANSIENT_ATTACHMENT_BIT = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	e_INPUT_ATTACHMENT_BIT = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
	e_SHADING_RATE_IMAGE_BIT_NV = VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV,
	e_FRAGMENT_DENSITY_MAP_BIT_EXT = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
	e_ALL_BITS = e_TRANSFER_SRC_BIT|e_TRANSFER_DST_BIT|e_SAMPLED_BIT|e_STORAGE_BIT|e_COLOR_ATTACHMENT_BIT|e_DEPTH_STENCIL_ATTACHMENT_BIT|e_TRANSIENT_ATTACHMENT_BIT|e_INPUT_ATTACHMENT_BIT|e_SHADING_RATE_IMAGE_BIT_NV|e_FRAGMENT_DENSITY_MAP_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ImageUsageFlags)
inline std::string to_string(ImageUsageFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ImageUsageFlags::e_TRANSFER_SRC_BIT, "VK_IMAGE_USAGE_TRANSFER_SRC_BIT");
	append_to_string_flag(value, returnString, ImageUsageFlags::e_TRANSFER_DST_BIT, "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
	append_to_string_flag(value, returnString, ImageUsageFlags::e_SAMPLED_BIT, "VK_IMAGE_USAGE_SAMPLED_BIT");
	append_to_string_flag(value, returnString, ImageUsageFlags::e_STORAGE_BIT, "VK_IMAGE_USAGE_STORAGE_BIT");
	append_to_string_flag(value, returnString, ImageUsageFlags::e_COLOR_ATTACHMENT_BIT, "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT");
	append_to_string_flag(value, returnString, ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT, "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT");
	append_to_string_flag(value, returnString, ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT");
	append_to_string_flag(value, returnString, ImageUsageFlags::e_INPUT_ATTACHMENT_BIT, "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT");
	append_to_string_flag(value, returnString, ImageUsageFlags::e_SHADING_RATE_IMAGE_BIT_NV, "VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV");
	append_to_string_flag(value, returnString, ImageUsageFlags::e_FRAGMENT_DENSITY_MAP_BIT_EXT, "VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT");
	return returnString;
}

enum class ImageCreateFlags
{
	e_NONE = 0,
	e_SPARSE_BINDING_BIT = VK_IMAGE_CREATE_SPARSE_BINDING_BIT,
	e_SPARSE_RESIDENCY_BIT = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT,
	e_SPARSE_ALIASED_BIT = VK_IMAGE_CREATE_SPARSE_ALIASED_BIT,
	e_MUTABLE_FORMAT_BIT = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
	e_CUBE_COMPATIBLE_BIT = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
	e_2D_ARRAY_COMPATIBLE_BIT = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT,
	e_SPLIT_INSTANCE_BIND_REGIONS_BIT = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT,
	e_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT,
	e_EXTENDED_USAGE_BIT = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT,
	e_DISJOINT_BIT = VK_IMAGE_CREATE_DISJOINT_BIT,
	e_ALIAS_BIT = VK_IMAGE_CREATE_ALIAS_BIT,
	e_PROTECTED_BIT = VK_IMAGE_CREATE_PROTECTED_BIT,
	e_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT = VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT,
	e_CORNER_SAMPLED_BIT_NV = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV,
	e_SUBSAMPLED_BIT_EXT = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT,
	e_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR,
	e_2D_ARRAY_COMPATIBLE_BIT_KHR = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR,
	e_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT_KHR = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT_KHR,
	e_EXTENDED_USAGE_BIT_KHR = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT_KHR,
	e_DISJOINT_BIT_KHR = VK_IMAGE_CREATE_DISJOINT_BIT_KHR,
	e_ALIAS_BIT_KHR = VK_IMAGE_CREATE_ALIAS_BIT_KHR,
	e_ALL_BITS = e_SPARSE_BINDING_BIT|e_SPARSE_RESIDENCY_BIT|e_SPARSE_ALIASED_BIT|e_MUTABLE_FORMAT_BIT|e_CUBE_COMPATIBLE_BIT|e_2D_ARRAY_COMPATIBLE_BIT|e_SPLIT_INSTANCE_BIND_REGIONS_BIT|e_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT|e_EXTENDED_USAGE_BIT|e_DISJOINT_BIT|e_ALIAS_BIT|e_PROTECTED_BIT|e_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT|e_CORNER_SAMPLED_BIT_NV|e_SUBSAMPLED_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ImageCreateFlags)
inline std::string to_string(ImageCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ImageCreateFlags::e_SPARSE_BINDING_BIT, "VK_IMAGE_CREATE_SPARSE_BINDING_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_SPARSE_RESIDENCY_BIT, "VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_SPARSE_ALIASED_BIT, "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_MUTABLE_FORMAT_BIT, "VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_CUBE_COMPATIBLE_BIT, "VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_2D_ARRAY_COMPATIBLE_BIT, "VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_SPLIT_INSTANCE_BIND_REGIONS_BIT, "VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, "VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_EXTENDED_USAGE_BIT, "VK_IMAGE_CREATE_EXTENDED_USAGE_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_DISJOINT_BIT, "VK_IMAGE_CREATE_DISJOINT_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_ALIAS_BIT, "VK_IMAGE_CREATE_ALIAS_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_PROTECTED_BIT, "VK_IMAGE_CREATE_PROTECTED_BIT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT, "VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_CORNER_SAMPLED_BIT_NV, "VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV");
	append_to_string_flag(value, returnString, ImageCreateFlags::e_SUBSAMPLED_BIT_EXT, "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
	return returnString;
}

enum class ImageViewCreateFlags
{
	e_NONE = 0,
	e_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT,
	e_ALL_BITS = e_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ImageViewCreateFlags)
inline std::string to_string(ImageViewCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ImageViewCreateFlags::e_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT, "VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT");
	return returnString;
}

enum class SamplerCreateFlags
{
	e_NONE = 0,
	e_SUBSAMPLED_BIT_EXT = VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT,
	e_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT = VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT,
	e_ALL_BITS = e_SUBSAMPLED_BIT_EXT|e_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SamplerCreateFlags)
inline std::string to_string(SamplerCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SamplerCreateFlags::e_SUBSAMPLED_BIT_EXT, "VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT");
	append_to_string_flag(value, returnString, SamplerCreateFlags::e_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT, "VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT");
	return returnString;
}

enum class PipelineCreateFlags
{
	e_NONE = 0,
	e_DISABLE_OPTIMIZATION_BIT = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
	e_ALLOW_DERIVATIVES_BIT = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
	e_DERIVATIVE_BIT = VK_PIPELINE_CREATE_DERIVATIVE_BIT,
	e_VIEW_INDEX_FROM_DEVICE_INDEX_BIT = VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT,
	e_DISPATCH_BASE_BIT = VK_PIPELINE_CREATE_DISPATCH_BASE_BIT,
	e_DEFER_COMPILE_BIT_NV = VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV,
	e_CAPTURE_STATISTICS_BIT_KHR = VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR,
	e_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR = VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR,
	e_DISPATCH_BASE = VK_PIPELINE_CREATE_DISPATCH_BASE,
	e_VIEW_INDEX_FROM_DEVICE_INDEX_BIT_KHR = VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT_KHR,
	e_DISPATCH_BASE_KHR = VK_PIPELINE_CREATE_DISPATCH_BASE_KHR,
	e_ALL_BITS = e_DISABLE_OPTIMIZATION_BIT|e_ALLOW_DERIVATIVES_BIT|e_DERIVATIVE_BIT|e_VIEW_INDEX_FROM_DEVICE_INDEX_BIT|e_DISPATCH_BASE_BIT|e_DEFER_COMPILE_BIT_NV|e_CAPTURE_STATISTICS_BIT_KHR|e_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PipelineCreateFlags)
inline std::string to_string(PipelineCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, PipelineCreateFlags::e_DISABLE_OPTIMIZATION_BIT, "VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT");
	append_to_string_flag(value, returnString, PipelineCreateFlags::e_ALLOW_DERIVATIVES_BIT, "VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT");
	append_to_string_flag(value, returnString, PipelineCreateFlags::e_DERIVATIVE_BIT, "VK_PIPELINE_CREATE_DERIVATIVE_BIT");
	append_to_string_flag(value, returnString, PipelineCreateFlags::e_VIEW_INDEX_FROM_DEVICE_INDEX_BIT, "VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT");
	append_to_string_flag(value, returnString, PipelineCreateFlags::e_DISPATCH_BASE_BIT, "VK_PIPELINE_CREATE_DISPATCH_BASE_BIT");
	append_to_string_flag(value, returnString, PipelineCreateFlags::e_DEFER_COMPILE_BIT_NV, "VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV");
	append_to_string_flag(value, returnString, PipelineCreateFlags::e_CAPTURE_STATISTICS_BIT_KHR, "VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR");
	append_to_string_flag(value, returnString, PipelineCreateFlags::e_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR, "VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR");
	return returnString;
}

enum class PipelineShaderStageCreateFlags
{
	e_NONE = 0,
	e_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT,
	e_REQUIRE_FULL_SUBGROUPS_BIT_EXT = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT,
	e_ALL_BITS = e_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT|e_REQUIRE_FULL_SUBGROUPS_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PipelineShaderStageCreateFlags)
inline std::string to_string(PipelineShaderStageCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, PipelineShaderStageCreateFlags::e_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT, "VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT");
	append_to_string_flag(value, returnString, PipelineShaderStageCreateFlags::e_REQUIRE_FULL_SUBGROUPS_BIT_EXT, "VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT");
	return returnString;
}

enum class ColorComponentFlags
{
	e_NONE = 0,
	e_R_BIT = VK_COLOR_COMPONENT_R_BIT,
	e_G_BIT = VK_COLOR_COMPONENT_G_BIT,
	e_B_BIT = VK_COLOR_COMPONENT_B_BIT,
	e_A_BIT = VK_COLOR_COMPONENT_A_BIT,
	e_ALL_BITS = e_R_BIT|e_G_BIT|e_B_BIT|e_A_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ColorComponentFlags)
inline std::string to_string(ColorComponentFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ColorComponentFlags::e_R_BIT, "VK_COLOR_COMPONENT_R_BIT");
	append_to_string_flag(value, returnString, ColorComponentFlags::e_G_BIT, "VK_COLOR_COMPONENT_G_BIT");
	append_to_string_flag(value, returnString, ColorComponentFlags::e_B_BIT, "VK_COLOR_COMPONENT_B_BIT");
	append_to_string_flag(value, returnString, ColorComponentFlags::e_A_BIT, "VK_COLOR_COMPONENT_A_BIT");
	return returnString;
}

enum class FenceCreateFlags
{
	e_NONE = 0,
	e_SIGNALED_BIT = VK_FENCE_CREATE_SIGNALED_BIT,
	e_ALL_BITS = e_SIGNALED_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(FenceCreateFlags)
inline std::string to_string(FenceCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, FenceCreateFlags::e_SIGNALED_BIT, "VK_FENCE_CREATE_SIGNALED_BIT");
	return returnString;
}

enum class SemaphoreCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(SemaphoreCreateFlags)
DEFINE_EMPTY_TO_STRING(SemaphoreCreateFlags)

enum class FormatFeatureFlags
{
	e_NONE = 0,
	e_SAMPLED_IMAGE_BIT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
	e_STORAGE_IMAGE_BIT = VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
	e_STORAGE_IMAGE_ATOMIC_BIT = VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT,
	e_UNIFORM_TEXEL_BUFFER_BIT = VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT,
	e_STORAGE_TEXEL_BUFFER_BIT = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT,
	e_STORAGE_TEXEL_BUFFER_ATOMIC_BIT = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT,
	e_VERTEX_BUFFER_BIT = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT,
	e_COLOR_ATTACHMENT_BIT = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,
	e_COLOR_ATTACHMENT_BLEND_BIT = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT,
	e_DEPTH_STENCIL_ATTACHMENT_BIT = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
	e_BLIT_SRC_BIT = VK_FORMAT_FEATURE_BLIT_SRC_BIT,
	e_BLIT_DST_BIT = VK_FORMAT_FEATURE_BLIT_DST_BIT,
	e_SAMPLED_IMAGE_FILTER_LINEAR_BIT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
	e_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG,
	e_TRANSFER_SRC_BIT = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT,
	e_TRANSFER_DST_BIT = VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
	e_SAMPLED_IMAGE_FILTER_MINMAX_BIT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT,
	e_MIDPOINT_CHROMA_SAMPLES_BIT = VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT,
	e_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT,
	e_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT,
	e_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
	e_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT,
	e_DISJOINT_BIT = VK_FORMAT_FEATURE_DISJOINT_BIT,
	e_COSITED_CHROMA_SAMPLES_BIT = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT,
	e_FRAGMENT_DENSITY_MAP_BIT_EXT = VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT,
	e_TRANSFER_SRC_BIT_KHR = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR,
	e_TRANSFER_DST_BIT_KHR = VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR,
	e_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT,
	e_MIDPOINT_CHROMA_SAMPLES_BIT_KHR = VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT_KHR,
	e_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT_KHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT_KHR,
	e_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT_KHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT_KHR,
	e_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT_KHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT_KHR,
	e_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT_KHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT_KHR,
	e_DISJOINT_BIT_KHR = VK_FORMAT_FEATURE_DISJOINT_BIT_KHR,
	e_COSITED_CHROMA_SAMPLES_BIT_KHR = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT_KHR,
	e_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT,
	e_ALL_BITS = e_SAMPLED_IMAGE_BIT|e_STORAGE_IMAGE_BIT|e_STORAGE_IMAGE_ATOMIC_BIT|e_UNIFORM_TEXEL_BUFFER_BIT|e_STORAGE_TEXEL_BUFFER_BIT|e_STORAGE_TEXEL_BUFFER_ATOMIC_BIT|e_VERTEX_BUFFER_BIT|e_COLOR_ATTACHMENT_BIT|e_COLOR_ATTACHMENT_BLEND_BIT|e_DEPTH_STENCIL_ATTACHMENT_BIT|e_BLIT_SRC_BIT|e_BLIT_DST_BIT|e_SAMPLED_IMAGE_FILTER_LINEAR_BIT|e_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG|e_TRANSFER_SRC_BIT|e_TRANSFER_DST_BIT|e_SAMPLED_IMAGE_FILTER_MINMAX_BIT|e_MIDPOINT_CHROMA_SAMPLES_BIT|e_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT|e_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT|e_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT|e_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT|e_DISJOINT_BIT|e_COSITED_CHROMA_SAMPLES_BIT|e_FRAGMENT_DENSITY_MAP_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(FormatFeatureFlags)
inline std::string to_string(FormatFeatureFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_SAMPLED_IMAGE_BIT, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_STORAGE_IMAGE_BIT, "VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_STORAGE_IMAGE_ATOMIC_BIT, "VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_UNIFORM_TEXEL_BUFFER_BIT, "VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_STORAGE_TEXEL_BUFFER_BIT, "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_STORAGE_TEXEL_BUFFER_ATOMIC_BIT, "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_VERTEX_BUFFER_BIT, "VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_COLOR_ATTACHMENT_BIT, "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_COLOR_ATTACHMENT_BLEND_BIT, "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT, "VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_BLIT_SRC_BIT, "VK_FORMAT_FEATURE_BLIT_SRC_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_BLIT_DST_BIT, "VK_FORMAT_FEATURE_BLIT_DST_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_SAMPLED_IMAGE_FILTER_LINEAR_BIT, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_TRANSFER_SRC_BIT, "VK_FORMAT_FEATURE_TRANSFER_SRC_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_TRANSFER_DST_BIT, "VK_FORMAT_FEATURE_TRANSFER_DST_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_SAMPLED_IMAGE_FILTER_MINMAX_BIT, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_MIDPOINT_CHROMA_SAMPLES_BIT, "VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_DISJOINT_BIT, "VK_FORMAT_FEATURE_DISJOINT_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_COSITED_CHROMA_SAMPLES_BIT, "VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT");
	append_to_string_flag(value, returnString, FormatFeatureFlags::e_FRAGMENT_DENSITY_MAP_BIT_EXT, "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT");
	return returnString;
}

enum class QueryControlFlags
{
	e_NONE = 0,
	e_PRECISE_BIT = VK_QUERY_CONTROL_PRECISE_BIT,
	e_ALL_BITS = e_PRECISE_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(QueryControlFlags)
inline std::string to_string(QueryControlFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, QueryControlFlags::e_PRECISE_BIT, "VK_QUERY_CONTROL_PRECISE_BIT");
	return returnString;
}

enum class QueryResultFlags
{
	e_NONE = 0,
	e_64_BIT = VK_QUERY_RESULT_64_BIT,
	e_WAIT_BIT = VK_QUERY_RESULT_WAIT_BIT,
	e_WITH_AVAILABILITY_BIT = VK_QUERY_RESULT_WITH_AVAILABILITY_BIT,
	e_PARTIAL_BIT = VK_QUERY_RESULT_PARTIAL_BIT,
	e_ALL_BITS = e_64_BIT|e_WAIT_BIT|e_WITH_AVAILABILITY_BIT|e_PARTIAL_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(QueryResultFlags)
inline std::string to_string(QueryResultFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, QueryResultFlags::e_64_BIT, "VK_QUERY_RESULT_64_BIT");
	append_to_string_flag(value, returnString, QueryResultFlags::e_WAIT_BIT, "VK_QUERY_RESULT_WAIT_BIT");
	append_to_string_flag(value, returnString, QueryResultFlags::e_WITH_AVAILABILITY_BIT, "VK_QUERY_RESULT_WITH_AVAILABILITY_BIT");
	append_to_string_flag(value, returnString, QueryResultFlags::e_PARTIAL_BIT, "VK_QUERY_RESULT_PARTIAL_BIT");
	return returnString;
}

enum class CommandBufferUsageFlags
{
	e_NONE = 0,
	e_ONE_TIME_SUBMIT_BIT = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	e_RENDER_PASS_CONTINUE_BIT = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
	e_SIMULTANEOUS_USE_BIT = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
	e_ALL_BITS = e_ONE_TIME_SUBMIT_BIT|e_RENDER_PASS_CONTINUE_BIT|e_SIMULTANEOUS_USE_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CommandBufferUsageFlags)
inline std::string to_string(CommandBufferUsageFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, CommandBufferUsageFlags::e_ONE_TIME_SUBMIT_BIT, "VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT");
	append_to_string_flag(value, returnString, CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT, "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT");
	append_to_string_flag(value, returnString, CommandBufferUsageFlags::e_SIMULTANEOUS_USE_BIT, "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT");
	return returnString;
}

enum class QueryPipelineStatisticFlags
{
	e_NONE = 0,
	e_INPUT_ASSEMBLY_VERTICES_BIT = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT,
	e_INPUT_ASSEMBLY_PRIMITIVES_BIT = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT,
	e_VERTEX_SHADER_INVOCATIONS_BIT = VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT,
	e_GEOMETRY_SHADER_INVOCATIONS_BIT = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT,
	e_GEOMETRY_SHADER_PRIMITIVES_BIT = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT,
	e_CLIPPING_INVOCATIONS_BIT = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT,
	e_CLIPPING_PRIMITIVES_BIT = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT,
	e_FRAGMENT_SHADER_INVOCATIONS_BIT = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT,
	e_TESSELLATION_CONTROL_SHADER_PATCHES_BIT = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT,
	e_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT,
	e_COMPUTE_SHADER_INVOCATIONS_BIT = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT,
	e_ALL_BITS = e_INPUT_ASSEMBLY_VERTICES_BIT|e_INPUT_ASSEMBLY_PRIMITIVES_BIT|e_VERTEX_SHADER_INVOCATIONS_BIT|e_GEOMETRY_SHADER_INVOCATIONS_BIT|e_GEOMETRY_SHADER_PRIMITIVES_BIT|e_CLIPPING_INVOCATIONS_BIT|e_CLIPPING_PRIMITIVES_BIT|e_FRAGMENT_SHADER_INVOCATIONS_BIT|e_TESSELLATION_CONTROL_SHADER_PATCHES_BIT|e_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT|e_COMPUTE_SHADER_INVOCATIONS_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(QueryPipelineStatisticFlags)
inline std::string to_string(QueryPipelineStatisticFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_INPUT_ASSEMBLY_VERTICES_BIT, "VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_INPUT_ASSEMBLY_PRIMITIVES_BIT, "VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_VERTEX_SHADER_INVOCATIONS_BIT, "VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_GEOMETRY_SHADER_INVOCATIONS_BIT, "VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_GEOMETRY_SHADER_PRIMITIVES_BIT, "VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_CLIPPING_INVOCATIONS_BIT, "VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_CLIPPING_PRIMITIVES_BIT, "VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_FRAGMENT_SHADER_INVOCATIONS_BIT, "VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_TESSELLATION_CONTROL_SHADER_PATCHES_BIT, "VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT, "VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT");
	append_to_string_flag(value, returnString, QueryPipelineStatisticFlags::e_COMPUTE_SHADER_INVOCATIONS_BIT, "VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT");
	return returnString;
}

enum class ImageAspectFlags
{
	e_NONE = 0,
	e_COLOR_BIT = VK_IMAGE_ASPECT_COLOR_BIT,
	e_DEPTH_BIT = VK_IMAGE_ASPECT_DEPTH_BIT,
	e_STENCIL_BIT = VK_IMAGE_ASPECT_STENCIL_BIT,
	e_METADATA_BIT = VK_IMAGE_ASPECT_METADATA_BIT,
	e_PLANE_0_BIT = VK_IMAGE_ASPECT_PLANE_0_BIT,
	e_PLANE_1_BIT = VK_IMAGE_ASPECT_PLANE_1_BIT,
	e_PLANE_2_BIT = VK_IMAGE_ASPECT_PLANE_2_BIT,
	e_MEMORY_PLANE_0_BIT_EXT = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT,
	e_MEMORY_PLANE_1_BIT_EXT = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
	e_MEMORY_PLANE_2_BIT_EXT = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT,
	e_MEMORY_PLANE_3_BIT_EXT = VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT,
	e_PLANE_0_BIT_KHR = VK_IMAGE_ASPECT_PLANE_0_BIT_KHR,
	e_PLANE_1_BIT_KHR = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR,
	e_PLANE_2_BIT_KHR = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR,
	e_ALL_BITS = e_COLOR_BIT|e_DEPTH_BIT|e_STENCIL_BIT|e_METADATA_BIT|e_PLANE_0_BIT|e_PLANE_1_BIT|e_PLANE_2_BIT|e_MEMORY_PLANE_0_BIT_EXT|e_MEMORY_PLANE_1_BIT_EXT|e_MEMORY_PLANE_2_BIT_EXT|e_MEMORY_PLANE_3_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ImageAspectFlags)
inline std::string to_string(ImageAspectFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ImageAspectFlags::e_COLOR_BIT, "VK_IMAGE_ASPECT_COLOR_BIT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_DEPTH_BIT, "VK_IMAGE_ASPECT_DEPTH_BIT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_STENCIL_BIT, "VK_IMAGE_ASPECT_STENCIL_BIT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_METADATA_BIT, "VK_IMAGE_ASPECT_METADATA_BIT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_PLANE_0_BIT, "VK_IMAGE_ASPECT_PLANE_0_BIT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_PLANE_1_BIT, "VK_IMAGE_ASPECT_PLANE_1_BIT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_PLANE_2_BIT, "VK_IMAGE_ASPECT_PLANE_2_BIT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_MEMORY_PLANE_0_BIT_EXT, "VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_MEMORY_PLANE_1_BIT_EXT, "VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_MEMORY_PLANE_2_BIT_EXT, "VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT");
	append_to_string_flag(value, returnString, ImageAspectFlags::e_MEMORY_PLANE_3_BIT_EXT, "VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT");
	return returnString;
}

enum class SparseImageFormatFlags
{
	e_NONE = 0,
	e_SINGLE_MIPTAIL_BIT = VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT,
	e_ALIGNED_MIP_SIZE_BIT = VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT,
	e_NONSTANDARD_BLOCK_SIZE_BIT = VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT,
	e_ALL_BITS = e_SINGLE_MIPTAIL_BIT|e_ALIGNED_MIP_SIZE_BIT|e_NONSTANDARD_BLOCK_SIZE_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SparseImageFormatFlags)
inline std::string to_string(SparseImageFormatFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SparseImageFormatFlags::e_SINGLE_MIPTAIL_BIT, "VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT");
	append_to_string_flag(value, returnString, SparseImageFormatFlags::e_ALIGNED_MIP_SIZE_BIT, "VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT");
	append_to_string_flag(value, returnString, SparseImageFormatFlags::e_NONSTANDARD_BLOCK_SIZE_BIT, "VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT");
	return returnString;
}

enum class SparseMemoryBindFlags
{
	e_NONE = 0,
	e_METADATA_BIT = VK_SPARSE_MEMORY_BIND_METADATA_BIT,
	e_ALL_BITS = e_METADATA_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SparseMemoryBindFlags)
inline std::string to_string(SparseMemoryBindFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SparseMemoryBindFlags::e_METADATA_BIT, "VK_SPARSE_MEMORY_BIND_METADATA_BIT");
	return returnString;
}

enum class PipelineStageFlags
{
	e_NONE = 0,
	e_TOP_OF_PIPE_BIT = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	e_DRAW_INDIRECT_BIT = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	e_VERTEX_INPUT_BIT = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	e_VERTEX_SHADER_BIT = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	e_TESSELLATION_CONTROL_SHADER_BIT = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
	e_TESSELLATION_EVALUATION_SHADER_BIT = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
	e_GEOMETRY_SHADER_BIT = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
	e_FRAGMENT_SHADER_BIT = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	e_EARLY_FRAGMENT_TESTS_BIT = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	e_LATE_FRAGMENT_TESTS_BIT = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
	e_COLOR_ATTACHMENT_OUTPUT_BIT = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	e_COMPUTE_SHADER_BIT = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	e_TRANSFER_BIT = VK_PIPELINE_STAGE_TRANSFER_BIT,
	e_BOTTOM_OF_PIPE_BIT = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
	e_HOST_BIT = VK_PIPELINE_STAGE_HOST_BIT,
	e_ALL_GRAPHICS_BIT = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
	e_ALL_COMMANDS_BIT = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	e_COMMAND_PROCESS_BIT_NVX = VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX,
	e_CONDITIONAL_RENDERING_BIT_EXT = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
	e_TASK_SHADER_BIT_NV = VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
	e_MESH_SHADER_BIT_NV = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
	e_RAY_TRACING_SHADER_BIT_NV = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
	e_SHADING_RATE_IMAGE_BIT_NV = VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV,
	e_FRAGMENT_DENSITY_PROCESS_BIT_EXT = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
	e_TRANSFORM_FEEDBACK_BIT_EXT = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
	e_ACCELERATION_STRUCTURE_BUILD_BIT_NV = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
	e_ALL_BITS = e_TOP_OF_PIPE_BIT|e_DRAW_INDIRECT_BIT|e_VERTEX_INPUT_BIT|e_VERTEX_SHADER_BIT|e_TESSELLATION_CONTROL_SHADER_BIT|e_TESSELLATION_EVALUATION_SHADER_BIT|e_GEOMETRY_SHADER_BIT|e_FRAGMENT_SHADER_BIT|e_EARLY_FRAGMENT_TESTS_BIT|e_LATE_FRAGMENT_TESTS_BIT|e_COLOR_ATTACHMENT_OUTPUT_BIT|e_COMPUTE_SHADER_BIT|e_TRANSFER_BIT|e_BOTTOM_OF_PIPE_BIT|e_HOST_BIT|e_ALL_GRAPHICS_BIT|e_ALL_COMMANDS_BIT|e_COMMAND_PROCESS_BIT_NVX|e_CONDITIONAL_RENDERING_BIT_EXT|e_TASK_SHADER_BIT_NV|e_MESH_SHADER_BIT_NV|e_RAY_TRACING_SHADER_BIT_NV|e_SHADING_RATE_IMAGE_BIT_NV|e_FRAGMENT_DENSITY_PROCESS_BIT_EXT|e_TRANSFORM_FEEDBACK_BIT_EXT|e_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PipelineStageFlags)
inline std::string to_string(PipelineStageFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, PipelineStageFlags::e_TOP_OF_PIPE_BIT, "VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_DRAW_INDIRECT_BIT, "VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_VERTEX_INPUT_BIT, "VK_PIPELINE_STAGE_VERTEX_INPUT_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_VERTEX_SHADER_BIT, "VK_PIPELINE_STAGE_VERTEX_SHADER_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_TESSELLATION_CONTROL_SHADER_BIT, "VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_TESSELLATION_EVALUATION_SHADER_BIT, "VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_GEOMETRY_SHADER_BIT, "VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_FRAGMENT_SHADER_BIT, "VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_EARLY_FRAGMENT_TESTS_BIT, "VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_LATE_FRAGMENT_TESTS_BIT, "VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, "VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_COMPUTE_SHADER_BIT, "VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_TRANSFER_BIT, "VK_PIPELINE_STAGE_TRANSFER_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT, "VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_HOST_BIT, "VK_PIPELINE_STAGE_HOST_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_ALL_GRAPHICS_BIT, "VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_ALL_COMMANDS_BIT, "VK_PIPELINE_STAGE_ALL_COMMANDS_BIT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_COMMAND_PROCESS_BIT_NVX, "VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_CONDITIONAL_RENDERING_BIT_EXT, "VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_TASK_SHADER_BIT_NV, "VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_MESH_SHADER_BIT_NV, "VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_RAY_TRACING_SHADER_BIT_NV, "VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_SHADING_RATE_IMAGE_BIT_NV, "VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_FRAGMENT_DENSITY_PROCESS_BIT_EXT, "VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_TRANSFORM_FEEDBACK_BIT_EXT, "VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT");
	append_to_string_flag(value, returnString, PipelineStageFlags::e_ACCELERATION_STRUCTURE_BUILD_BIT_NV, "VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV");
	return returnString;
}

enum class CommandPoolCreateFlags
{
	e_NONE = 0,
	e_TRANSIENT_BIT = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
	e_RESET_COMMAND_BUFFER_BIT = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	e_PROTECTED_BIT = VK_COMMAND_POOL_CREATE_PROTECTED_BIT,
	e_ALL_BITS = e_TRANSIENT_BIT|e_RESET_COMMAND_BUFFER_BIT|e_PROTECTED_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CommandPoolCreateFlags)
inline std::string to_string(CommandPoolCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, CommandPoolCreateFlags::e_TRANSIENT_BIT, "VK_COMMAND_POOL_CREATE_TRANSIENT_BIT");
	append_to_string_flag(value, returnString, CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT, "VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT");
	append_to_string_flag(value, returnString, CommandPoolCreateFlags::e_PROTECTED_BIT, "VK_COMMAND_POOL_CREATE_PROTECTED_BIT");
	return returnString;
}

enum class CommandPoolResetFlags
{
	e_NONE = 0,
	e_RELEASE_RESOURCES_BIT = VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT,
	e_ALL_BITS = e_RELEASE_RESOURCES_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CommandPoolResetFlags)
inline std::string to_string(CommandPoolResetFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, CommandPoolResetFlags::e_RELEASE_RESOURCES_BIT, "VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT");
	return returnString;
}

enum class CommandBufferResetFlags
{
	e_NONE = 0,
	e_RELEASE_RESOURCES_BIT = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT,
	e_ALL_BITS = e_RELEASE_RESOURCES_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CommandBufferResetFlags)
inline std::string to_string(CommandBufferResetFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, CommandBufferResetFlags::e_RELEASE_RESOURCES_BIT, "VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT");
	return returnString;
}

enum class SampleCountFlags
{
	e_NONE = 0,
	e_1_BIT = VK_SAMPLE_COUNT_1_BIT,
	e_2_BIT = VK_SAMPLE_COUNT_2_BIT,
	e_4_BIT = VK_SAMPLE_COUNT_4_BIT,
	e_8_BIT = VK_SAMPLE_COUNT_8_BIT,
	e_16_BIT = VK_SAMPLE_COUNT_16_BIT,
	e_32_BIT = VK_SAMPLE_COUNT_32_BIT,
	e_64_BIT = VK_SAMPLE_COUNT_64_BIT,
	e_ALL_BITS = e_1_BIT|e_2_BIT|e_4_BIT|e_8_BIT|e_16_BIT|e_32_BIT|e_64_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SampleCountFlags)
inline std::string to_string(SampleCountFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SampleCountFlags::e_1_BIT, "VK_SAMPLE_COUNT_1_BIT");
	append_to_string_flag(value, returnString, SampleCountFlags::e_2_BIT, "VK_SAMPLE_COUNT_2_BIT");
	append_to_string_flag(value, returnString, SampleCountFlags::e_4_BIT, "VK_SAMPLE_COUNT_4_BIT");
	append_to_string_flag(value, returnString, SampleCountFlags::e_8_BIT, "VK_SAMPLE_COUNT_8_BIT");
	append_to_string_flag(value, returnString, SampleCountFlags::e_16_BIT, "VK_SAMPLE_COUNT_16_BIT");
	append_to_string_flag(value, returnString, SampleCountFlags::e_32_BIT, "VK_SAMPLE_COUNT_32_BIT");
	append_to_string_flag(value, returnString, SampleCountFlags::e_64_BIT, "VK_SAMPLE_COUNT_64_BIT");
	return returnString;
}

enum class AttachmentDescriptionFlags
{
	e_NONE = 0,
	e_MAY_ALIAS_BIT = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT,
	e_ALL_BITS = e_MAY_ALIAS_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(AttachmentDescriptionFlags)
inline std::string to_string(AttachmentDescriptionFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, AttachmentDescriptionFlags::e_MAY_ALIAS_BIT, "VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT");
	return returnString;
}

enum class StencilFaceFlags
{
	e_NONE = 0,
	e_FRONT_BIT = VK_STENCIL_FACE_FRONT_BIT,
	e_BACK_BIT = VK_STENCIL_FACE_BACK_BIT,
	e_FRONT_AND_BACK = VK_STENCIL_FACE_FRONT_AND_BACK,
	e_ALL_BITS = e_FRONT_BIT|e_BACK_BIT|e_FRONT_AND_BACK,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(StencilFaceFlags)
inline std::string to_string(StencilFaceFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, StencilFaceFlags::e_FRONT_BIT, "VK_STENCIL_FACE_FRONT_BIT");
	append_to_string_flag(value, returnString, StencilFaceFlags::e_BACK_BIT, "VK_STENCIL_FACE_BACK_BIT");
	append_to_string_flag(value, returnString, StencilFaceFlags::e_FRONT_AND_BACK, "VK_STENCIL_FACE_FRONT_AND_BACK");
	return returnString;
}

enum class DescriptorPoolCreateFlags
{
	e_NONE = 0,
	e_FREE_DESCRIPTOR_SET_BIT = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
	e_UPDATE_AFTER_BIND_BIT = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
	e_UPDATE_AFTER_BIND_BIT_EXT = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
	e_ALL_BITS = e_FREE_DESCRIPTOR_SET_BIT|e_UPDATE_AFTER_BIND_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DescriptorPoolCreateFlags)
inline std::string to_string(DescriptorPoolCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DescriptorPoolCreateFlags::e_FREE_DESCRIPTOR_SET_BIT, "VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT");
	append_to_string_flag(value, returnString, DescriptorPoolCreateFlags::e_UPDATE_AFTER_BIND_BIT, "VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT");
	return returnString;
}

enum class DependencyFlags
{
	e_NONE = 0,
	e_BY_REGION_BIT = VK_DEPENDENCY_BY_REGION_BIT,
	e_VIEW_LOCAL_BIT = VK_DEPENDENCY_VIEW_LOCAL_BIT,
	e_DEVICE_GROUP_BIT = VK_DEPENDENCY_DEVICE_GROUP_BIT,
	e_VIEW_LOCAL_BIT_KHR = VK_DEPENDENCY_VIEW_LOCAL_BIT_KHR,
	e_DEVICE_GROUP_BIT_KHR = VK_DEPENDENCY_DEVICE_GROUP_BIT_KHR,
	e_ALL_BITS = e_BY_REGION_BIT|e_VIEW_LOCAL_BIT|e_DEVICE_GROUP_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DependencyFlags)
inline std::string to_string(DependencyFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DependencyFlags::e_BY_REGION_BIT, "VK_DEPENDENCY_BY_REGION_BIT");
	append_to_string_flag(value, returnString, DependencyFlags::e_VIEW_LOCAL_BIT, "VK_DEPENDENCY_VIEW_LOCAL_BIT");
	append_to_string_flag(value, returnString, DependencyFlags::e_DEVICE_GROUP_BIT, "VK_DEPENDENCY_DEVICE_GROUP_BIT");
	return returnString;
}

enum class SemaphoreWaitFlags
{
	e_NONE = 0,
	e_ANY_BIT = VK_SEMAPHORE_WAIT_ANY_BIT,
	e_ANY_BIT_KHR = VK_SEMAPHORE_WAIT_ANY_BIT_KHR,
	e_ALL_BITS = e_ANY_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SemaphoreWaitFlags)
inline std::string to_string(SemaphoreWaitFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SemaphoreWaitFlags::e_ANY_BIT, "VK_SEMAPHORE_WAIT_ANY_BIT");
	return returnString;
}

enum class DisplayPlaneAlphaFlagsKHR
{
	e_NONE = 0,
	e_OPAQUE_BIT_KHR = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
	e_GLOBAL_BIT_KHR = VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR,
	e_PER_PIXEL_BIT_KHR = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR,
	e_PER_PIXEL_PREMULTIPLIED_BIT_KHR = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR,
	e_ALL_BITS = e_OPAQUE_BIT_KHR|e_GLOBAL_BIT_KHR|e_PER_PIXEL_BIT_KHR|e_PER_PIXEL_PREMULTIPLIED_BIT_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DisplayPlaneAlphaFlagsKHR)
inline std::string to_string(DisplayPlaneAlphaFlagsKHR value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DisplayPlaneAlphaFlagsKHR::e_OPAQUE_BIT_KHR, "VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR");
	append_to_string_flag(value, returnString, DisplayPlaneAlphaFlagsKHR::e_GLOBAL_BIT_KHR, "VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR");
	append_to_string_flag(value, returnString, DisplayPlaneAlphaFlagsKHR::e_PER_PIXEL_BIT_KHR, "VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR");
	append_to_string_flag(value, returnString, DisplayPlaneAlphaFlagsKHR::e_PER_PIXEL_PREMULTIPLIED_BIT_KHR, "VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR");
	return returnString;
}

enum class CompositeAlphaFlagsKHR
{
	e_NONE = 0,
	e_OPAQUE_BIT_KHR = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	e_PRE_MULTIPLIED_BIT_KHR = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
	e_POST_MULTIPLIED_BIT_KHR = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
	e_INHERIT_BIT_KHR = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	e_ALL_BITS = e_OPAQUE_BIT_KHR|e_PRE_MULTIPLIED_BIT_KHR|e_POST_MULTIPLIED_BIT_KHR|e_INHERIT_BIT_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CompositeAlphaFlagsKHR)
inline std::string to_string(CompositeAlphaFlagsKHR value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, CompositeAlphaFlagsKHR::e_OPAQUE_BIT_KHR, "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR");
	append_to_string_flag(value, returnString, CompositeAlphaFlagsKHR::e_PRE_MULTIPLIED_BIT_KHR, "VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR");
	append_to_string_flag(value, returnString, CompositeAlphaFlagsKHR::e_POST_MULTIPLIED_BIT_KHR, "VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR");
	append_to_string_flag(value, returnString, CompositeAlphaFlagsKHR::e_INHERIT_BIT_KHR, "VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR");
	return returnString;
}

enum class SurfaceTransformFlagsKHR
{
	e_NONE = 0,
	e_IDENTITY_BIT_KHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
	e_ROTATE_90_BIT_KHR = VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR,
	e_ROTATE_180_BIT_KHR = VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR,
	e_ROTATE_270_BIT_KHR = VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR,
	e_HORIZONTAL_MIRROR_BIT_KHR = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR,
	e_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,
	e_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR,
	e_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR,
	e_INHERIT_BIT_KHR = VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR,
	e_ALL_BITS = e_IDENTITY_BIT_KHR|e_ROTATE_90_BIT_KHR|e_ROTATE_180_BIT_KHR|e_ROTATE_270_BIT_KHR|e_HORIZONTAL_MIRROR_BIT_KHR|e_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR|e_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR|e_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR|e_INHERIT_BIT_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SurfaceTransformFlagsKHR)
inline std::string to_string(SurfaceTransformFlagsKHR value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR, "VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR");
	append_to_string_flag(value, returnString, SurfaceTransformFlagsKHR::e_ROTATE_90_BIT_KHR, "VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR");
	append_to_string_flag(value, returnString, SurfaceTransformFlagsKHR::e_ROTATE_180_BIT_KHR, "VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR");
	append_to_string_flag(value, returnString, SurfaceTransformFlagsKHR::e_ROTATE_270_BIT_KHR, "VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR");
	append_to_string_flag(value, returnString, SurfaceTransformFlagsKHR::e_HORIZONTAL_MIRROR_BIT_KHR, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR");
	append_to_string_flag(value, returnString, SurfaceTransformFlagsKHR::e_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR");
	append_to_string_flag(value, returnString, SurfaceTransformFlagsKHR::e_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR");
	append_to_string_flag(value, returnString, SurfaceTransformFlagsKHR::e_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR");
	append_to_string_flag(value, returnString, SurfaceTransformFlagsKHR::e_INHERIT_BIT_KHR, "VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR");
	return returnString;
}

enum class DebugReportFlagsEXT
{
	e_NONE = 0,
	e_INFORMATION_BIT_EXT = VK_DEBUG_REPORT_INFORMATION_BIT_EXT,
	e_WARNING_BIT_EXT = VK_DEBUG_REPORT_WARNING_BIT_EXT,
	e_PERFORMANCE_WARNING_BIT_EXT = VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
	e_ERROR_BIT_EXT = VK_DEBUG_REPORT_ERROR_BIT_EXT,
	e_DEBUG_BIT_EXT = VK_DEBUG_REPORT_DEBUG_BIT_EXT,
	e_ALL_BITS = e_INFORMATION_BIT_EXT|e_WARNING_BIT_EXT|e_PERFORMANCE_WARNING_BIT_EXT|e_ERROR_BIT_EXT|e_DEBUG_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DebugReportFlagsEXT)
inline std::string to_string(DebugReportFlagsEXT value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DebugReportFlagsEXT::e_INFORMATION_BIT_EXT, "VK_DEBUG_REPORT_INFORMATION_BIT_EXT");
	append_to_string_flag(value, returnString, DebugReportFlagsEXT::e_WARNING_BIT_EXT, "VK_DEBUG_REPORT_WARNING_BIT_EXT");
	append_to_string_flag(value, returnString, DebugReportFlagsEXT::e_PERFORMANCE_WARNING_BIT_EXT, "VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT");
	append_to_string_flag(value, returnString, DebugReportFlagsEXT::e_ERROR_BIT_EXT, "VK_DEBUG_REPORT_ERROR_BIT_EXT");
	append_to_string_flag(value, returnString, DebugReportFlagsEXT::e_DEBUG_BIT_EXT, "VK_DEBUG_REPORT_DEBUG_BIT_EXT");
	return returnString;
}

enum class ExternalMemoryHandleTypeFlagsNV
{
	e_NONE = 0,
	e_OPAQUE_WIN32_BIT_NV = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_NV,
	e_OPAQUE_WIN32_KMT_BIT_NV = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_NV,
	e_D3D11_IMAGE_BIT_NV = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_IMAGE_BIT_NV,
	e_D3D11_IMAGE_KMT_BIT_NV = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_IMAGE_KMT_BIT_NV,
	e_ALL_BITS = e_OPAQUE_WIN32_BIT_NV|e_OPAQUE_WIN32_KMT_BIT_NV|e_D3D11_IMAGE_BIT_NV|e_D3D11_IMAGE_KMT_BIT_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ExternalMemoryHandleTypeFlagsNV)
inline std::string to_string(ExternalMemoryHandleTypeFlagsNV value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlagsNV::e_OPAQUE_WIN32_BIT_NV, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_NV");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlagsNV::e_OPAQUE_WIN32_KMT_BIT_NV, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_NV");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlagsNV::e_D3D11_IMAGE_BIT_NV, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_IMAGE_BIT_NV");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlagsNV::e_D3D11_IMAGE_KMT_BIT_NV, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_IMAGE_KMT_BIT_NV");
	return returnString;
}

enum class ExternalMemoryFeatureFlagsNV
{
	e_NONE = 0,
	e_DEDICATED_ONLY_BIT_NV = VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT_NV,
	e_EXPORTABLE_BIT_NV = VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_NV,
	e_IMPORTABLE_BIT_NV = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_NV,
	e_ALL_BITS = e_DEDICATED_ONLY_BIT_NV|e_EXPORTABLE_BIT_NV|e_IMPORTABLE_BIT_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ExternalMemoryFeatureFlagsNV)
inline std::string to_string(ExternalMemoryFeatureFlagsNV value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ExternalMemoryFeatureFlagsNV::e_DEDICATED_ONLY_BIT_NV, "VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT_NV");
	append_to_string_flag(value, returnString, ExternalMemoryFeatureFlagsNV::e_EXPORTABLE_BIT_NV, "VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_NV");
	append_to_string_flag(value, returnString, ExternalMemoryFeatureFlagsNV::e_IMPORTABLE_BIT_NV, "VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_NV");
	return returnString;
}

enum class SubgroupFeatureFlags
{
	e_NONE = 0,
	e_BASIC_BIT = VK_SUBGROUP_FEATURE_BASIC_BIT,
	e_VOTE_BIT = VK_SUBGROUP_FEATURE_VOTE_BIT,
	e_ARITHMETIC_BIT = VK_SUBGROUP_FEATURE_ARITHMETIC_BIT,
	e_BALLOT_BIT = VK_SUBGROUP_FEATURE_BALLOT_BIT,
	e_SHUFFLE_BIT = VK_SUBGROUP_FEATURE_SHUFFLE_BIT,
	e_SHUFFLE_RELATIVE_BIT = VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT,
	e_CLUSTERED_BIT = VK_SUBGROUP_FEATURE_CLUSTERED_BIT,
	e_QUAD_BIT = VK_SUBGROUP_FEATURE_QUAD_BIT,
	e_PARTITIONED_BIT_NV = VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV,
	e_ALL_BITS = e_BASIC_BIT|e_VOTE_BIT|e_ARITHMETIC_BIT|e_BALLOT_BIT|e_SHUFFLE_BIT|e_SHUFFLE_RELATIVE_BIT|e_CLUSTERED_BIT|e_QUAD_BIT|e_PARTITIONED_BIT_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SubgroupFeatureFlags)
inline std::string to_string(SubgroupFeatureFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SubgroupFeatureFlags::e_BASIC_BIT, "VK_SUBGROUP_FEATURE_BASIC_BIT");
	append_to_string_flag(value, returnString, SubgroupFeatureFlags::e_VOTE_BIT, "VK_SUBGROUP_FEATURE_VOTE_BIT");
	append_to_string_flag(value, returnString, SubgroupFeatureFlags::e_ARITHMETIC_BIT, "VK_SUBGROUP_FEATURE_ARITHMETIC_BIT");
	append_to_string_flag(value, returnString, SubgroupFeatureFlags::e_BALLOT_BIT, "VK_SUBGROUP_FEATURE_BALLOT_BIT");
	append_to_string_flag(value, returnString, SubgroupFeatureFlags::e_SHUFFLE_BIT, "VK_SUBGROUP_FEATURE_SHUFFLE_BIT");
	append_to_string_flag(value, returnString, SubgroupFeatureFlags::e_SHUFFLE_RELATIVE_BIT, "VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT");
	append_to_string_flag(value, returnString, SubgroupFeatureFlags::e_CLUSTERED_BIT, "VK_SUBGROUP_FEATURE_CLUSTERED_BIT");
	append_to_string_flag(value, returnString, SubgroupFeatureFlags::e_QUAD_BIT, "VK_SUBGROUP_FEATURE_QUAD_BIT");
	append_to_string_flag(value, returnString, SubgroupFeatureFlags::e_PARTITIONED_BIT_NV, "VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV");
	return returnString;
}

enum class IndirectCommandsLayoutUsageFlagsNVX
{
	e_NONE = 0,
	e_UNORDERED_SEQUENCES_BIT_NVX = VK_INDIRECT_COMMANDS_LAYOUT_USAGE_UNORDERED_SEQUENCES_BIT_NVX,
	e_SPARSE_SEQUENCES_BIT_NVX = VK_INDIRECT_COMMANDS_LAYOUT_USAGE_SPARSE_SEQUENCES_BIT_NVX,
	e_EMPTY_EXECUTIONS_BIT_NVX = VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EMPTY_EXECUTIONS_BIT_NVX,
	e_INDEXED_SEQUENCES_BIT_NVX = VK_INDIRECT_COMMANDS_LAYOUT_USAGE_INDEXED_SEQUENCES_BIT_NVX,
	e_ALL_BITS = e_UNORDERED_SEQUENCES_BIT_NVX|e_SPARSE_SEQUENCES_BIT_NVX|e_EMPTY_EXECUTIONS_BIT_NVX|e_INDEXED_SEQUENCES_BIT_NVX,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(IndirectCommandsLayoutUsageFlagsNVX)
inline std::string to_string(IndirectCommandsLayoutUsageFlagsNVX value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, IndirectCommandsLayoutUsageFlagsNVX::e_UNORDERED_SEQUENCES_BIT_NVX, "VK_INDIRECT_COMMANDS_LAYOUT_USAGE_UNORDERED_SEQUENCES_BIT_NVX");
	append_to_string_flag(value, returnString, IndirectCommandsLayoutUsageFlagsNVX::e_SPARSE_SEQUENCES_BIT_NVX, "VK_INDIRECT_COMMANDS_LAYOUT_USAGE_SPARSE_SEQUENCES_BIT_NVX");
	append_to_string_flag(value, returnString, IndirectCommandsLayoutUsageFlagsNVX::e_EMPTY_EXECUTIONS_BIT_NVX, "VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EMPTY_EXECUTIONS_BIT_NVX");
	append_to_string_flag(value, returnString, IndirectCommandsLayoutUsageFlagsNVX::e_INDEXED_SEQUENCES_BIT_NVX, "VK_INDIRECT_COMMANDS_LAYOUT_USAGE_INDEXED_SEQUENCES_BIT_NVX");
	return returnString;
}

enum class ObjectEntryUsageFlagsNVX
{
	e_NONE = 0,
	e_GRAPHICS_BIT_NVX = VK_OBJECT_ENTRY_USAGE_GRAPHICS_BIT_NVX,
	e_COMPUTE_BIT_NVX = VK_OBJECT_ENTRY_USAGE_COMPUTE_BIT_NVX,
	e_ALL_BITS = e_GRAPHICS_BIT_NVX|e_COMPUTE_BIT_NVX,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ObjectEntryUsageFlagsNVX)
inline std::string to_string(ObjectEntryUsageFlagsNVX value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ObjectEntryUsageFlagsNVX::e_GRAPHICS_BIT_NVX, "VK_OBJECT_ENTRY_USAGE_GRAPHICS_BIT_NVX");
	append_to_string_flag(value, returnString, ObjectEntryUsageFlagsNVX::e_COMPUTE_BIT_NVX, "VK_OBJECT_ENTRY_USAGE_COMPUTE_BIT_NVX");
	return returnString;
}

enum class DescriptorSetLayoutCreateFlags
{
	e_NONE = 0,
	e_PUSH_DESCRIPTOR_BIT_KHR = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
	e_UPDATE_AFTER_BIND_POOL_BIT = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
	e_UPDATE_AFTER_BIND_POOL_BIT_EXT = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
	e_ALL_BITS = e_PUSH_DESCRIPTOR_BIT_KHR|e_UPDATE_AFTER_BIND_POOL_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DescriptorSetLayoutCreateFlags)
inline std::string to_string(DescriptorSetLayoutCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DescriptorSetLayoutCreateFlags::e_PUSH_DESCRIPTOR_BIT_KHR, "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR");
	append_to_string_flag(value, returnString, DescriptorSetLayoutCreateFlags::e_UPDATE_AFTER_BIND_POOL_BIT, "VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT");
	return returnString;
}

enum class ExternalMemoryHandleTypeFlags
{
	e_NONE = 0,
	e_OPAQUE_FD_BIT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT,
	e_OPAQUE_WIN32_BIT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT,
	e_OPAQUE_WIN32_KMT_BIT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT,
	e_D3D11_TEXTURE_BIT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT,
	e_D3D11_TEXTURE_KMT_BIT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT,
	e_D3D12_HEAP_BIT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT,
	e_D3D12_RESOURCE_BIT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT,
	e_HOST_ALLOCATION_BIT_EXT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT,
	e_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT,
	e_DMA_BUF_BIT_EXT = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
	e_ANDROID_HARDWARE_BUFFER_BIT_ANDROID = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
	e_OPAQUE_FD_BIT_KHR = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR,
	e_OPAQUE_WIN32_BIT_KHR = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR,
	e_OPAQUE_WIN32_KMT_BIT_KHR = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR,
	e_D3D11_TEXTURE_BIT_KHR = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT_KHR,
	e_D3D11_TEXTURE_KMT_BIT_KHR = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT_KHR,
	e_D3D12_HEAP_BIT_KHR = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT_KHR,
	e_D3D12_RESOURCE_BIT_KHR = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT_KHR,
	e_ALL_BITS = e_OPAQUE_FD_BIT|e_OPAQUE_WIN32_BIT|e_OPAQUE_WIN32_KMT_BIT|e_D3D11_TEXTURE_BIT|e_D3D11_TEXTURE_KMT_BIT|e_D3D12_HEAP_BIT|e_D3D12_RESOURCE_BIT|e_HOST_ALLOCATION_BIT_EXT|e_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT|e_DMA_BUF_BIT_EXT|e_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ExternalMemoryHandleTypeFlags)
inline std::string to_string(ExternalMemoryHandleTypeFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_OPAQUE_FD_BIT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_OPAQUE_WIN32_BIT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_OPAQUE_WIN32_KMT_BIT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_D3D11_TEXTURE_BIT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_D3D11_TEXTURE_KMT_BIT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_D3D12_HEAP_BIT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_D3D12_RESOURCE_BIT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_HOST_ALLOCATION_BIT_EXT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_DMA_BUF_BIT_EXT, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT");
	append_to_string_flag(value, returnString, ExternalMemoryHandleTypeFlags::e_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID");
	return returnString;
}

enum class ExternalMemoryFeatureFlags
{
	e_NONE = 0,
	e_DEDICATED_ONLY_BIT = VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT,
	e_EXPORTABLE_BIT = VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT,
	e_IMPORTABLE_BIT = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT,
	e_DEDICATED_ONLY_BIT_KHR = VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT_KHR,
	e_EXPORTABLE_BIT_KHR = VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR,
	e_IMPORTABLE_BIT_KHR = VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR,
	e_ALL_BITS = e_DEDICATED_ONLY_BIT|e_EXPORTABLE_BIT|e_IMPORTABLE_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ExternalMemoryFeatureFlags)
inline std::string to_string(ExternalMemoryFeatureFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ExternalMemoryFeatureFlags::e_DEDICATED_ONLY_BIT, "VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT");
	append_to_string_flag(value, returnString, ExternalMemoryFeatureFlags::e_EXPORTABLE_BIT, "VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT");
	append_to_string_flag(value, returnString, ExternalMemoryFeatureFlags::e_IMPORTABLE_BIT, "VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT");
	return returnString;
}

enum class ExternalSemaphoreHandleTypeFlags
{
	e_NONE = 0,
	e_OPAQUE_FD_BIT = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT,
	e_OPAQUE_WIN32_BIT = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
	e_OPAQUE_WIN32_KMT_BIT = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT,
	e_D3D12_FENCE_BIT = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT,
	e_SYNC_FD_BIT = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT,
	e_OPAQUE_FD_BIT_KHR = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR,
	e_OPAQUE_WIN32_BIT_KHR = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR,
	e_OPAQUE_WIN32_KMT_BIT_KHR = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR,
	e_D3D12_FENCE_BIT_KHR = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT_KHR,
	e_SYNC_FD_BIT_KHR = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR,
	e_ALL_BITS = e_OPAQUE_FD_BIT|e_OPAQUE_WIN32_BIT|e_OPAQUE_WIN32_KMT_BIT|e_D3D12_FENCE_BIT|e_SYNC_FD_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ExternalSemaphoreHandleTypeFlags)
inline std::string to_string(ExternalSemaphoreHandleTypeFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ExternalSemaphoreHandleTypeFlags::e_OPAQUE_FD_BIT, "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT");
	append_to_string_flag(value, returnString, ExternalSemaphoreHandleTypeFlags::e_OPAQUE_WIN32_BIT, "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT");
	append_to_string_flag(value, returnString, ExternalSemaphoreHandleTypeFlags::e_OPAQUE_WIN32_KMT_BIT, "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT");
	append_to_string_flag(value, returnString, ExternalSemaphoreHandleTypeFlags::e_D3D12_FENCE_BIT, "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT");
	append_to_string_flag(value, returnString, ExternalSemaphoreHandleTypeFlags::e_SYNC_FD_BIT, "VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT");
	return returnString;
}

enum class ExternalSemaphoreFeatureFlags
{
	e_NONE = 0,
	e_EXPORTABLE_BIT = VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT,
	e_IMPORTABLE_BIT = VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT,
	e_EXPORTABLE_BIT_KHR = VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR,
	e_IMPORTABLE_BIT_KHR = VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR,
	e_ALL_BITS = e_EXPORTABLE_BIT|e_IMPORTABLE_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ExternalSemaphoreFeatureFlags)
inline std::string to_string(ExternalSemaphoreFeatureFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ExternalSemaphoreFeatureFlags::e_EXPORTABLE_BIT, "VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT");
	append_to_string_flag(value, returnString, ExternalSemaphoreFeatureFlags::e_IMPORTABLE_BIT, "VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT");
	return returnString;
}

enum class SemaphoreImportFlags
{
	e_NONE = 0,
	e_TEMPORARY_BIT = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT,
	e_TEMPORARY_BIT_KHR = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR,
	e_ALL_BITS = e_TEMPORARY_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SemaphoreImportFlags)
inline std::string to_string(SemaphoreImportFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SemaphoreImportFlags::e_TEMPORARY_BIT, "VK_SEMAPHORE_IMPORT_TEMPORARY_BIT");
	return returnString;
}

enum class ExternalFenceHandleTypeFlags
{
	e_NONE = 0,
	e_OPAQUE_FD_BIT = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT,
	e_OPAQUE_WIN32_BIT = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
	e_OPAQUE_WIN32_KMT_BIT = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT,
	e_SYNC_FD_BIT = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT,
	e_OPAQUE_FD_BIT_KHR = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR,
	e_OPAQUE_WIN32_BIT_KHR = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR,
	e_OPAQUE_WIN32_KMT_BIT_KHR = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR,
	e_SYNC_FD_BIT_KHR = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR,
	e_ALL_BITS = e_OPAQUE_FD_BIT|e_OPAQUE_WIN32_BIT|e_OPAQUE_WIN32_KMT_BIT|e_SYNC_FD_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ExternalFenceHandleTypeFlags)
inline std::string to_string(ExternalFenceHandleTypeFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ExternalFenceHandleTypeFlags::e_OPAQUE_FD_BIT, "VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT");
	append_to_string_flag(value, returnString, ExternalFenceHandleTypeFlags::e_OPAQUE_WIN32_BIT, "VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT");
	append_to_string_flag(value, returnString, ExternalFenceHandleTypeFlags::e_OPAQUE_WIN32_KMT_BIT, "VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT");
	append_to_string_flag(value, returnString, ExternalFenceHandleTypeFlags::e_SYNC_FD_BIT, "VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT");
	return returnString;
}

enum class ExternalFenceFeatureFlags
{
	e_NONE = 0,
	e_EXPORTABLE_BIT = VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT,
	e_IMPORTABLE_BIT = VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT,
	e_EXPORTABLE_BIT_KHR = VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR,
	e_IMPORTABLE_BIT_KHR = VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR,
	e_ALL_BITS = e_EXPORTABLE_BIT|e_IMPORTABLE_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ExternalFenceFeatureFlags)
inline std::string to_string(ExternalFenceFeatureFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ExternalFenceFeatureFlags::e_EXPORTABLE_BIT, "VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT");
	append_to_string_flag(value, returnString, ExternalFenceFeatureFlags::e_IMPORTABLE_BIT, "VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT");
	return returnString;
}

enum class FenceImportFlags
{
	e_NONE = 0,
	e_TEMPORARY_BIT = VK_FENCE_IMPORT_TEMPORARY_BIT,
	e_TEMPORARY_BIT_KHR = VK_FENCE_IMPORT_TEMPORARY_BIT_KHR,
	e_ALL_BITS = e_TEMPORARY_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(FenceImportFlags)
inline std::string to_string(FenceImportFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, FenceImportFlags::e_TEMPORARY_BIT, "VK_FENCE_IMPORT_TEMPORARY_BIT");
	return returnString;
}

enum class SurfaceCounterFlagsEXT
{
	e_NONE = 0,
	e_VBLANK_EXT = VK_SURFACE_COUNTER_VBLANK_EXT,
	e_ALL_BITS = e_VBLANK_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SurfaceCounterFlagsEXT)
inline std::string to_string(SurfaceCounterFlagsEXT value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SurfaceCounterFlagsEXT::e_VBLANK_EXT, "VK_SURFACE_COUNTER_VBLANK_EXT");
	return returnString;
}

enum class PeerMemoryFeatureFlags
{
	e_NONE = 0,
	e_COPY_SRC_BIT = VK_PEER_MEMORY_FEATURE_COPY_SRC_BIT,
	e_COPY_DST_BIT = VK_PEER_MEMORY_FEATURE_COPY_DST_BIT,
	e_GENERIC_SRC_BIT = VK_PEER_MEMORY_FEATURE_GENERIC_SRC_BIT,
	e_GENERIC_DST_BIT = VK_PEER_MEMORY_FEATURE_GENERIC_DST_BIT,
	e_COPY_SRC_BIT_KHR = VK_PEER_MEMORY_FEATURE_COPY_SRC_BIT_KHR,
	e_COPY_DST_BIT_KHR = VK_PEER_MEMORY_FEATURE_COPY_DST_BIT_KHR,
	e_GENERIC_SRC_BIT_KHR = VK_PEER_MEMORY_FEATURE_GENERIC_SRC_BIT_KHR,
	e_GENERIC_DST_BIT_KHR = VK_PEER_MEMORY_FEATURE_GENERIC_DST_BIT_KHR,
	e_ALL_BITS = e_COPY_SRC_BIT|e_COPY_DST_BIT|e_GENERIC_SRC_BIT|e_GENERIC_DST_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PeerMemoryFeatureFlags)
inline std::string to_string(PeerMemoryFeatureFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, PeerMemoryFeatureFlags::e_COPY_SRC_BIT, "VK_PEER_MEMORY_FEATURE_COPY_SRC_BIT");
	append_to_string_flag(value, returnString, PeerMemoryFeatureFlags::e_COPY_DST_BIT, "VK_PEER_MEMORY_FEATURE_COPY_DST_BIT");
	append_to_string_flag(value, returnString, PeerMemoryFeatureFlags::e_GENERIC_SRC_BIT, "VK_PEER_MEMORY_FEATURE_GENERIC_SRC_BIT");
	append_to_string_flag(value, returnString, PeerMemoryFeatureFlags::e_GENERIC_DST_BIT, "VK_PEER_MEMORY_FEATURE_GENERIC_DST_BIT");
	return returnString;
}

enum class MemoryAllocateFlags
{
	e_NONE = 0,
	e_DEVICE_MASK_BIT = VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT,
	e_DEVICE_ADDRESS_BIT = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
	e_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT,
	e_DEVICE_MASK_BIT_KHR = VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT_KHR,
	e_DEVICE_ADDRESS_BIT_KHR = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
	e_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR,
	e_ALL_BITS = e_DEVICE_MASK_BIT|e_DEVICE_ADDRESS_BIT|e_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(MemoryAllocateFlags)
inline std::string to_string(MemoryAllocateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, MemoryAllocateFlags::e_DEVICE_MASK_BIT, "VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT");
	append_to_string_flag(value, returnString, MemoryAllocateFlags::e_DEVICE_ADDRESS_BIT, "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT");
	append_to_string_flag(value, returnString, MemoryAllocateFlags::e_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT, "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT");
	return returnString;
}

enum class DeviceGroupPresentModeFlagsKHR
{
	e_NONE = 0,
	e_LOCAL_BIT_KHR = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR,
	e_REMOTE_BIT_KHR = VK_DEVICE_GROUP_PRESENT_MODE_REMOTE_BIT_KHR,
	e_SUM_BIT_KHR = VK_DEVICE_GROUP_PRESENT_MODE_SUM_BIT_KHR,
	e_LOCAL_MULTI_DEVICE_BIT_KHR = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_MULTI_DEVICE_BIT_KHR,
	e_ALL_BITS = e_LOCAL_BIT_KHR|e_REMOTE_BIT_KHR|e_SUM_BIT_KHR|e_LOCAL_MULTI_DEVICE_BIT_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DeviceGroupPresentModeFlagsKHR)
inline std::string to_string(DeviceGroupPresentModeFlagsKHR value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DeviceGroupPresentModeFlagsKHR::e_LOCAL_BIT_KHR, "VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR");
	append_to_string_flag(value, returnString, DeviceGroupPresentModeFlagsKHR::e_REMOTE_BIT_KHR, "VK_DEVICE_GROUP_PRESENT_MODE_REMOTE_BIT_KHR");
	append_to_string_flag(value, returnString, DeviceGroupPresentModeFlagsKHR::e_SUM_BIT_KHR, "VK_DEVICE_GROUP_PRESENT_MODE_SUM_BIT_KHR");
	append_to_string_flag(value, returnString, DeviceGroupPresentModeFlagsKHR::e_LOCAL_MULTI_DEVICE_BIT_KHR, "VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_MULTI_DEVICE_BIT_KHR");
	return returnString;
}

enum class SwapchainCreateFlagsKHR
{
	e_NONE = 0,
	e_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR = VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR,
	e_PROTECTED_BIT_KHR = VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR,
	e_MUTABLE_FORMAT_BIT_KHR = VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR,
	e_ALL_BITS = e_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR|e_PROTECTED_BIT_KHR|e_MUTABLE_FORMAT_BIT_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SwapchainCreateFlagsKHR)
inline std::string to_string(SwapchainCreateFlagsKHR value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SwapchainCreateFlagsKHR::e_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR, "VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR");
	append_to_string_flag(value, returnString, SwapchainCreateFlagsKHR::e_PROTECTED_BIT_KHR, "VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR");
	append_to_string_flag(value, returnString, SwapchainCreateFlagsKHR::e_MUTABLE_FORMAT_BIT_KHR, "VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR");
	return returnString;
}

enum class SubpassDescriptionFlags
{
	e_NONE = 0,
	e_PER_VIEW_ATTRIBUTES_BIT_NVX = VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX,
	e_PER_VIEW_POSITION_X_ONLY_BIT_NVX = VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX,
	e_ALL_BITS = e_PER_VIEW_ATTRIBUTES_BIT_NVX|e_PER_VIEW_POSITION_X_ONLY_BIT_NVX,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SubpassDescriptionFlags)
inline std::string to_string(SubpassDescriptionFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, SubpassDescriptionFlags::e_PER_VIEW_ATTRIBUTES_BIT_NVX, "VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX");
	append_to_string_flag(value, returnString, SubpassDescriptionFlags::e_PER_VIEW_POSITION_X_ONLY_BIT_NVX, "VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX");
	return returnString;
}

enum class DebugUtilsMessageSeverityFlagsEXT
{
	e_NONE = 0,
	e_VERBOSE_BIT_EXT = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
	e_INFO_BIT_EXT = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
	e_WARNING_BIT_EXT = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
	e_ERROR_BIT_EXT = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	e_ALL_BITS = e_VERBOSE_BIT_EXT|e_INFO_BIT_EXT|e_WARNING_BIT_EXT|e_ERROR_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DebugUtilsMessageSeverityFlagsEXT)
inline std::string to_string(DebugUtilsMessageSeverityFlagsEXT value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DebugUtilsMessageSeverityFlagsEXT::e_VERBOSE_BIT_EXT, "VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT");
	append_to_string_flag(value, returnString, DebugUtilsMessageSeverityFlagsEXT::e_INFO_BIT_EXT, "VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT");
	append_to_string_flag(value, returnString, DebugUtilsMessageSeverityFlagsEXT::e_WARNING_BIT_EXT, "VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT");
	append_to_string_flag(value, returnString, DebugUtilsMessageSeverityFlagsEXT::e_ERROR_BIT_EXT, "VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT");
	return returnString;
}

enum class DebugUtilsMessageTypeFlagsEXT
{
	e_NONE = 0,
	e_GENERAL_BIT_EXT = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
	e_VALIDATION_BIT_EXT = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
	e_PERFORMANCE_BIT_EXT = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	e_ALL_BITS = e_GENERAL_BIT_EXT|e_VALIDATION_BIT_EXT|e_PERFORMANCE_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DebugUtilsMessageTypeFlagsEXT)
inline std::string to_string(DebugUtilsMessageTypeFlagsEXT value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DebugUtilsMessageTypeFlagsEXT::e_GENERAL_BIT_EXT, "VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT");
	append_to_string_flag(value, returnString, DebugUtilsMessageTypeFlagsEXT::e_VALIDATION_BIT_EXT, "VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT");
	append_to_string_flag(value, returnString, DebugUtilsMessageTypeFlagsEXT::e_PERFORMANCE_BIT_EXT, "VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT");
	return returnString;
}

enum class DescriptorBindingFlags
{
	e_NONE = 0,
	e_UPDATE_AFTER_BIND_BIT = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
	e_UPDATE_UNUSED_WHILE_PENDING_BIT = VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT,
	e_PARTIALLY_BOUND_BIT = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
	e_VARIABLE_DESCRIPTOR_COUNT_BIT = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
	e_UPDATE_AFTER_BIND_BIT_EXT = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
	e_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT = VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT,
	e_PARTIALLY_BOUND_BIT_EXT = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT,
	e_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT,
	e_ALL_BITS = e_UPDATE_AFTER_BIND_BIT|e_UPDATE_UNUSED_WHILE_PENDING_BIT|e_PARTIALLY_BOUND_BIT|e_VARIABLE_DESCRIPTOR_COUNT_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DescriptorBindingFlags)
inline std::string to_string(DescriptorBindingFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, DescriptorBindingFlags::e_UPDATE_AFTER_BIND_BIT, "VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT");
	append_to_string_flag(value, returnString, DescriptorBindingFlags::e_UPDATE_UNUSED_WHILE_PENDING_BIT, "VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT");
	append_to_string_flag(value, returnString, DescriptorBindingFlags::e_PARTIALLY_BOUND_BIT, "VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT");
	append_to_string_flag(value, returnString, DescriptorBindingFlags::e_VARIABLE_DESCRIPTOR_COUNT_BIT, "VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT");
	return returnString;
}

enum class ConditionalRenderingFlagsEXT
{
	e_NONE = 0,
	e_INVERTED_BIT_EXT = VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT,
	e_ALL_BITS = e_INVERTED_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ConditionalRenderingFlagsEXT)
inline std::string to_string(ConditionalRenderingFlagsEXT value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ConditionalRenderingFlagsEXT::e_INVERTED_BIT_EXT, "VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT");
	return returnString;
}

enum class ResolveModeFlags
{
	e_NONE = VK_RESOLVE_MODE_NONE,
	e_SAMPLE_ZERO_BIT = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT,
	e_AVERAGE_BIT = VK_RESOLVE_MODE_AVERAGE_BIT,
	e_MIN_BIT = VK_RESOLVE_MODE_MIN_BIT,
	e_MAX_BIT = VK_RESOLVE_MODE_MAX_BIT,
	e_NONE_KHR = VK_RESOLVE_MODE_NONE_KHR,
	e_SAMPLE_ZERO_BIT_KHR = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR,
	e_AVERAGE_BIT_KHR = VK_RESOLVE_MODE_AVERAGE_BIT_KHR,
	e_MIN_BIT_KHR = VK_RESOLVE_MODE_MIN_BIT_KHR,
	e_MAX_BIT_KHR = VK_RESOLVE_MODE_MAX_BIT_KHR,
	e_ALL_BITS = e_SAMPLE_ZERO_BIT|e_AVERAGE_BIT|e_MIN_BIT|e_MAX_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ResolveModeFlags)
inline std::string to_string(ResolveModeFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ResolveModeFlags::e_NONE, "VK_RESOLVE_MODE_NONE");
	append_to_string_flag(value, returnString, ResolveModeFlags::e_SAMPLE_ZERO_BIT, "VK_RESOLVE_MODE_SAMPLE_ZERO_BIT");
	append_to_string_flag(value, returnString, ResolveModeFlags::e_AVERAGE_BIT, "VK_RESOLVE_MODE_AVERAGE_BIT");
	append_to_string_flag(value, returnString, ResolveModeFlags::e_MIN_BIT, "VK_RESOLVE_MODE_MIN_BIT");
	append_to_string_flag(value, returnString, ResolveModeFlags::e_MAX_BIT, "VK_RESOLVE_MODE_MAX_BIT");
	return returnString;
}

enum class GeometryInstanceFlagsNV
{
	e_NONE = 0,
	e_TRIANGLE_CULL_DISABLE_BIT_NV = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
	e_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV,
	e_FORCE_OPAQUE_BIT_NV = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV,
	e_FORCE_NO_OPAQUE_BIT_NV = VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV,
	e_ALL_BITS = e_TRIANGLE_CULL_DISABLE_BIT_NV|e_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV|e_FORCE_OPAQUE_BIT_NV|e_FORCE_NO_OPAQUE_BIT_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(GeometryInstanceFlagsNV)
inline std::string to_string(GeometryInstanceFlagsNV value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, GeometryInstanceFlagsNV::e_TRIANGLE_CULL_DISABLE_BIT_NV, "VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV");
	append_to_string_flag(value, returnString, GeometryInstanceFlagsNV::e_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV, "VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV");
	append_to_string_flag(value, returnString, GeometryInstanceFlagsNV::e_FORCE_OPAQUE_BIT_NV, "VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV");
	append_to_string_flag(value, returnString, GeometryInstanceFlagsNV::e_FORCE_NO_OPAQUE_BIT_NV, "VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV");
	return returnString;
}

enum class GeometryFlagsNV
{
	e_NONE = 0,
	e_OPAQUE_BIT_NV = VK_GEOMETRY_OPAQUE_BIT_NV,
	e_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV,
	e_ALL_BITS = e_OPAQUE_BIT_NV|e_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(GeometryFlagsNV)
inline std::string to_string(GeometryFlagsNV value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, GeometryFlagsNV::e_OPAQUE_BIT_NV, "VK_GEOMETRY_OPAQUE_BIT_NV");
	append_to_string_flag(value, returnString, GeometryFlagsNV::e_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV, "VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV");
	return returnString;
}

enum class BuildAccelerationStructureFlagsNV
{
	e_NONE = 0,
	e_ALLOW_UPDATE_BIT_NV = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV,
	e_ALLOW_COMPACTION_BIT_NV = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV,
	e_PREFER_FAST_TRACE_BIT_NV = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV,
	e_PREFER_FAST_BUILD_BIT_NV = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV,
	e_LOW_MEMORY_BIT_NV = VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV,
	e_ALL_BITS = e_ALLOW_UPDATE_BIT_NV|e_ALLOW_COMPACTION_BIT_NV|e_PREFER_FAST_TRACE_BIT_NV|e_PREFER_FAST_BUILD_BIT_NV|e_LOW_MEMORY_BIT_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(BuildAccelerationStructureFlagsNV)
inline std::string to_string(BuildAccelerationStructureFlagsNV value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, BuildAccelerationStructureFlagsNV::e_ALLOW_UPDATE_BIT_NV, "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV");
	append_to_string_flag(value, returnString, BuildAccelerationStructureFlagsNV::e_ALLOW_COMPACTION_BIT_NV, "VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV");
	append_to_string_flag(value, returnString, BuildAccelerationStructureFlagsNV::e_PREFER_FAST_TRACE_BIT_NV, "VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV");
	append_to_string_flag(value, returnString, BuildAccelerationStructureFlagsNV::e_PREFER_FAST_BUILD_BIT_NV, "VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV");
	append_to_string_flag(value, returnString, BuildAccelerationStructureFlagsNV::e_LOW_MEMORY_BIT_NV, "VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV");
	return returnString;
}

enum class FramebufferCreateFlags
{
	e_NONE = 0,
	e_IMAGELESS_BIT = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
	e_IMAGELESS_BIT_KHR = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR,
	e_ALL_BITS = e_IMAGELESS_BIT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(FramebufferCreateFlags)
inline std::string to_string(FramebufferCreateFlags value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, FramebufferCreateFlags::e_IMAGELESS_BIT, "VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT");
	return returnString;
}

enum class PipelineCreationFeedbackFlagsEXT
{
	e_NONE = 0,
	e_VALID_BIT_EXT = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT_EXT,
	e_APPLICATION_PIPELINE_CACHE_HIT_BIT_EXT = VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT_EXT,
	e_BASE_PIPELINE_ACCELERATION_BIT_EXT = VK_PIPELINE_CREATION_FEEDBACK_BASE_PIPELINE_ACCELERATION_BIT_EXT,
	e_ALL_BITS = e_VALID_BIT_EXT|e_APPLICATION_PIPELINE_CACHE_HIT_BIT_EXT|e_BASE_PIPELINE_ACCELERATION_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PipelineCreationFeedbackFlagsEXT)
inline std::string to_string(PipelineCreationFeedbackFlagsEXT value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, PipelineCreationFeedbackFlagsEXT::e_VALID_BIT_EXT, "VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT_EXT");
	append_to_string_flag(value, returnString, PipelineCreationFeedbackFlagsEXT::e_APPLICATION_PIPELINE_CACHE_HIT_BIT_EXT, "VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT_EXT");
	append_to_string_flag(value, returnString, PipelineCreationFeedbackFlagsEXT::e_BASE_PIPELINE_ACCELERATION_BIT_EXT, "VK_PIPELINE_CREATION_FEEDBACK_BASE_PIPELINE_ACCELERATION_BIT_EXT");
	return returnString;
}

enum class PerformanceCounterDescriptionFlagsKHR
{
	e_NONE = 0,
	e_PERFORMANCE_IMPACTING_KHR = VK_PERFORMANCE_COUNTER_DESCRIPTION_PERFORMANCE_IMPACTING_KHR,
	e_CONCURRENTLY_IMPACTED_KHR = VK_PERFORMANCE_COUNTER_DESCRIPTION_CONCURRENTLY_IMPACTED_KHR,
	e_ALL_BITS = e_PERFORMANCE_IMPACTING_KHR|e_CONCURRENTLY_IMPACTED_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PerformanceCounterDescriptionFlagsKHR)
inline std::string to_string(PerformanceCounterDescriptionFlagsKHR value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, PerformanceCounterDescriptionFlagsKHR::e_PERFORMANCE_IMPACTING_KHR, "VK_PERFORMANCE_COUNTER_DESCRIPTION_PERFORMANCE_IMPACTING_KHR");
	append_to_string_flag(value, returnString, PerformanceCounterDescriptionFlagsKHR::e_CONCURRENTLY_IMPACTED_KHR, "VK_PERFORMANCE_COUNTER_DESCRIPTION_CONCURRENTLY_IMPACTED_KHR");
	return returnString;
}

enum class AcquireProfilingLockFlagsKHR: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(AcquireProfilingLockFlagsKHR)
DEFINE_EMPTY_TO_STRING(AcquireProfilingLockFlagsKHR)

enum class ShaderCorePropertiesFlagsAMD: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(ShaderCorePropertiesFlagsAMD)
DEFINE_EMPTY_TO_STRING(ShaderCorePropertiesFlagsAMD)

enum class ShaderModuleCreateFlags: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(ShaderModuleCreateFlags)
DEFINE_EMPTY_TO_STRING(ShaderModuleCreateFlags)

enum class PipelineCompilerControlFlagsAMD: uint32_t { e_NONE };
DEFINE_ENUM_BITWISE_OPERATORS(PipelineCompilerControlFlagsAMD)
DEFINE_EMPTY_TO_STRING(PipelineCompilerControlFlagsAMD)

enum class ToolPurposeFlagsEXT
{
	e_NONE = 0,
	e_VALIDATION_BIT_EXT = VK_TOOL_PURPOSE_VALIDATION_BIT_EXT,
	e_PROFILING_BIT_EXT = VK_TOOL_PURPOSE_PROFILING_BIT_EXT,
	e_TRACING_BIT_EXT = VK_TOOL_PURPOSE_TRACING_BIT_EXT,
	e_ADDITIONAL_FEATURES_BIT_EXT = VK_TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT_EXT,
	e_MODIFYING_FEATURES_BIT_EXT = VK_TOOL_PURPOSE_MODIFYING_FEATURES_BIT_EXT,
	e_DEBUG_REPORTING_BIT_EXT = VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT,
	e_DEBUG_MARKERS_BIT_EXT = VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT,
	e_ALL_BITS = e_VALIDATION_BIT_EXT|e_PROFILING_BIT_EXT|e_TRACING_BIT_EXT|e_ADDITIONAL_FEATURES_BIT_EXT|e_MODIFYING_FEATURES_BIT_EXT|e_DEBUG_REPORTING_BIT_EXT|e_DEBUG_MARKERS_BIT_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ToolPurposeFlagsEXT)
inline std::string to_string(ToolPurposeFlagsEXT value)
{
	std::string returnString = "";
	append_to_string_flag(value, returnString, ToolPurposeFlagsEXT::e_VALIDATION_BIT_EXT, "VK_TOOL_PURPOSE_VALIDATION_BIT_EXT");
	append_to_string_flag(value, returnString, ToolPurposeFlagsEXT::e_PROFILING_BIT_EXT, "VK_TOOL_PURPOSE_PROFILING_BIT_EXT");
	append_to_string_flag(value, returnString, ToolPurposeFlagsEXT::e_TRACING_BIT_EXT, "VK_TOOL_PURPOSE_TRACING_BIT_EXT");
	append_to_string_flag(value, returnString, ToolPurposeFlagsEXT::e_ADDITIONAL_FEATURES_BIT_EXT, "VK_TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT_EXT");
	append_to_string_flag(value, returnString, ToolPurposeFlagsEXT::e_MODIFYING_FEATURES_BIT_EXT, "VK_TOOL_PURPOSE_MODIFYING_FEATURES_BIT_EXT");
	append_to_string_flag(value, returnString, ToolPurposeFlagsEXT::e_DEBUG_REPORTING_BIT_EXT, "VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT");
	append_to_string_flag(value, returnString, ToolPurposeFlagsEXT::e_DEBUG_MARKERS_BIT_EXT, "VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT");
	return returnString;
}


// PVRVk Enums
enum class ImageLayout
{
	e_UNDEFINED = VK_IMAGE_LAYOUT_UNDEFINED,
	e_GENERAL = VK_IMAGE_LAYOUT_GENERAL,
	e_COLOR_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	e_DEPTH_STENCIL_READ_ONLY_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
	e_SHADER_READ_ONLY_OPTIMAL = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	e_TRANSFER_SRC_OPTIMAL = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	e_TRANSFER_DST_OPTIMAL = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	e_PREINITIALIZED = VK_IMAGE_LAYOUT_PREINITIALIZED,
	e_PRESENT_SRC_KHR = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	e_SHARED_PRESENT_KHR = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
	e_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
	e_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
	e_SHADING_RATE_OPTIMAL_NV = VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
	e_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
	e_DEPTH_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
	e_DEPTH_READ_ONLY_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
	e_STENCIL_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
	e_STENCIL_READ_ONLY_OPTIMAL = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
	e_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR,
	e_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR,
	e_DEPTH_ATTACHMENT_OPTIMAL_KHR = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR,
	e_DEPTH_READ_ONLY_OPTIMAL_KHR = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR,
	e_STENCIL_ATTACHMENT_OPTIMAL_KHR = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR,
	e_STENCIL_READ_ONLY_OPTIMAL_KHR = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR,
	e_BEGIN_RANGE = VK_IMAGE_LAYOUT_BEGIN_RANGE,
	e_END_RANGE = VK_IMAGE_LAYOUT_END_RANGE,
	e_RANGE_SIZE = VK_IMAGE_LAYOUT_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ImageLayout)
inline std::string to_string(ImageLayout value)
{
	switch(value)
	{
	case ImageLayout::e_UNDEFINED: return "VK_IMAGE_LAYOUT_UNDEFINED";
	case ImageLayout::e_GENERAL: return "VK_IMAGE_LAYOUT_GENERAL";
	case ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL: return "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL";
	case ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
	case ImageLayout::e_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL";
	case ImageLayout::e_SHADER_READ_ONLY_OPTIMAL: return "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL";
	case ImageLayout::e_TRANSFER_SRC_OPTIMAL: return "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL";
	case ImageLayout::e_TRANSFER_DST_OPTIMAL: return "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL";
	case ImageLayout::e_PREINITIALIZED: return "VK_IMAGE_LAYOUT_PREINITIALIZED";
	case ImageLayout::e_PRESENT_SRC_KHR: return "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR";
	case ImageLayout::e_SHARED_PRESENT_KHR: return "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR";
	case ImageLayout::e_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
	case ImageLayout::e_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
	case ImageLayout::e_SHADING_RATE_OPTIMAL_NV: return "VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV";
	case ImageLayout::e_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT: return "VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT";
	case ImageLayout::e_DEPTH_ATTACHMENT_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL";
	case ImageLayout::e_DEPTH_READ_ONLY_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL";
	case ImageLayout::e_STENCIL_ATTACHMENT_OPTIMAL: return "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL";
	case ImageLayout::e_STENCIL_READ_ONLY_OPTIMAL: return "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL";
	default: return "invalid";
	}
}

enum class AttachmentLoadOp
{
	e_LOAD = VK_ATTACHMENT_LOAD_OP_LOAD,
	e_CLEAR = VK_ATTACHMENT_LOAD_OP_CLEAR,
	e_DONT_CARE = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	e_BEGIN_RANGE = VK_ATTACHMENT_LOAD_OP_BEGIN_RANGE,
	e_END_RANGE = VK_ATTACHMENT_LOAD_OP_END_RANGE,
	e_RANGE_SIZE = VK_ATTACHMENT_LOAD_OP_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(AttachmentLoadOp)
inline std::string to_string(AttachmentLoadOp value)
{
	switch(value)
	{
	case AttachmentLoadOp::e_LOAD: return "VK_ATTACHMENT_LOAD_OP_LOAD";
	case AttachmentLoadOp::e_CLEAR: return "VK_ATTACHMENT_LOAD_OP_CLEAR";
	case AttachmentLoadOp::e_DONT_CARE: return "VK_ATTACHMENT_LOAD_OP_DONT_CARE";
	default: return "invalid";
	}
}

enum class AttachmentStoreOp
{
	e_STORE = VK_ATTACHMENT_STORE_OP_STORE,
	e_DONT_CARE = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	e_BEGIN_RANGE = VK_ATTACHMENT_STORE_OP_BEGIN_RANGE,
	e_END_RANGE = VK_ATTACHMENT_STORE_OP_END_RANGE,
	e_RANGE_SIZE = VK_ATTACHMENT_STORE_OP_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(AttachmentStoreOp)
inline std::string to_string(AttachmentStoreOp value)
{
	switch(value)
	{
	case AttachmentStoreOp::e_STORE: return "VK_ATTACHMENT_STORE_OP_STORE";
	case AttachmentStoreOp::e_DONT_CARE: return "VK_ATTACHMENT_STORE_OP_DONT_CARE";
	default: return "invalid";
	}
}

enum class ImageType
{
	e_1D = VK_IMAGE_TYPE_1D,
	e_2D = VK_IMAGE_TYPE_2D,
	e_3D = VK_IMAGE_TYPE_3D,
	e_BEGIN_RANGE = VK_IMAGE_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_IMAGE_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_IMAGE_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ImageType)
inline std::string to_string(ImageType value)
{
	switch(value)
	{
	case ImageType::e_1D: return "VK_IMAGE_TYPE_1D";
	case ImageType::e_2D: return "VK_IMAGE_TYPE_2D";
	case ImageType::e_3D: return "VK_IMAGE_TYPE_3D";
	default: return "invalid";
	}
}

enum class ImageTiling
{
	e_OPTIMAL = VK_IMAGE_TILING_OPTIMAL,
	e_LINEAR = VK_IMAGE_TILING_LINEAR,
	e_DRM_FORMAT_MODIFIER_EXT = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT,
	e_BEGIN_RANGE = VK_IMAGE_TILING_BEGIN_RANGE,
	e_END_RANGE = VK_IMAGE_TILING_END_RANGE,
	e_RANGE_SIZE = VK_IMAGE_TILING_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ImageTiling)
inline std::string to_string(ImageTiling value)
{
	switch(value)
	{
	case ImageTiling::e_OPTIMAL: return "VK_IMAGE_TILING_OPTIMAL";
	case ImageTiling::e_LINEAR: return "VK_IMAGE_TILING_LINEAR";
	case ImageTiling::e_DRM_FORMAT_MODIFIER_EXT: return "VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT";
	default: return "invalid";
	}
}

enum class ImageViewType
{
	e_1D = VK_IMAGE_VIEW_TYPE_1D,
	e_2D = VK_IMAGE_VIEW_TYPE_2D,
	e_3D = VK_IMAGE_VIEW_TYPE_3D,
	e_CUBE = VK_IMAGE_VIEW_TYPE_CUBE,
	e_1D_ARRAY = VK_IMAGE_VIEW_TYPE_1D_ARRAY,
	e_2D_ARRAY = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
	e_CUBE_ARRAY = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
	e_BEGIN_RANGE = VK_IMAGE_VIEW_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_IMAGE_VIEW_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_IMAGE_VIEW_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ImageViewType)
inline std::string to_string(ImageViewType value)
{
	switch(value)
	{
	case ImageViewType::e_1D: return "VK_IMAGE_VIEW_TYPE_1D";
	case ImageViewType::e_2D: return "VK_IMAGE_VIEW_TYPE_2D";
	case ImageViewType::e_3D: return "VK_IMAGE_VIEW_TYPE_3D";
	case ImageViewType::e_CUBE: return "VK_IMAGE_VIEW_TYPE_CUBE";
	case ImageViewType::e_1D_ARRAY: return "VK_IMAGE_VIEW_TYPE_1D_ARRAY";
	case ImageViewType::e_2D_ARRAY: return "VK_IMAGE_VIEW_TYPE_2D_ARRAY";
	case ImageViewType::e_CUBE_ARRAY: return "VK_IMAGE_VIEW_TYPE_CUBE_ARRAY";
	default: return "invalid";
	}
}

enum class CommandBufferLevel
{
	e_PRIMARY = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	e_SECONDARY = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
	e_BEGIN_RANGE = VK_COMMAND_BUFFER_LEVEL_BEGIN_RANGE,
	e_END_RANGE = VK_COMMAND_BUFFER_LEVEL_END_RANGE,
	e_RANGE_SIZE = VK_COMMAND_BUFFER_LEVEL_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CommandBufferLevel)
inline std::string to_string(CommandBufferLevel value)
{
	switch(value)
	{
	case CommandBufferLevel::e_PRIMARY: return "VK_COMMAND_BUFFER_LEVEL_PRIMARY";
	case CommandBufferLevel::e_SECONDARY: return "VK_COMMAND_BUFFER_LEVEL_SECONDARY";
	default: return "invalid";
	}
}

enum class ComponentSwizzle
{
	e_IDENTITY = VK_COMPONENT_SWIZZLE_IDENTITY,
	e_ZERO = VK_COMPONENT_SWIZZLE_ZERO,
	e_ONE = VK_COMPONENT_SWIZZLE_ONE,
	e_R = VK_COMPONENT_SWIZZLE_R,
	e_G = VK_COMPONENT_SWIZZLE_G,
	e_B = VK_COMPONENT_SWIZZLE_B,
	e_A = VK_COMPONENT_SWIZZLE_A,
	e_BEGIN_RANGE = VK_COMPONENT_SWIZZLE_BEGIN_RANGE,
	e_END_RANGE = VK_COMPONENT_SWIZZLE_END_RANGE,
	e_RANGE_SIZE = VK_COMPONENT_SWIZZLE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ComponentSwizzle)
inline std::string to_string(ComponentSwizzle value)
{
	switch(value)
	{
	case ComponentSwizzle::e_IDENTITY: return "VK_COMPONENT_SWIZZLE_IDENTITY";
	case ComponentSwizzle::e_ZERO: return "VK_COMPONENT_SWIZZLE_ZERO";
	case ComponentSwizzle::e_ONE: return "VK_COMPONENT_SWIZZLE_ONE";
	case ComponentSwizzle::e_R: return "VK_COMPONENT_SWIZZLE_R";
	case ComponentSwizzle::e_G: return "VK_COMPONENT_SWIZZLE_G";
	case ComponentSwizzle::e_B: return "VK_COMPONENT_SWIZZLE_B";
	case ComponentSwizzle::e_A: return "VK_COMPONENT_SWIZZLE_A";
	default: return "invalid";
	}
}

enum class DescriptorType
{
	e_SAMPLER = VK_DESCRIPTOR_TYPE_SAMPLER,
	e_COMBINED_IMAGE_SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	e_SAMPLED_IMAGE = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	e_STORAGE_IMAGE = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	e_UNIFORM_TEXEL_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
	e_STORAGE_TEXEL_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
	e_UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	e_STORAGE_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	e_UNIFORM_BUFFER_DYNAMIC = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	e_STORAGE_BUFFER_DYNAMIC = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
	e_INPUT_ATTACHMENT = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
	e_INLINE_UNIFORM_BLOCK_EXT = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT,
	e_ACCELERATION_STRUCTURE_NV = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
	e_BEGIN_RANGE = VK_DESCRIPTOR_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_DESCRIPTOR_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_DESCRIPTOR_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DescriptorType)
inline std::string to_string(DescriptorType value)
{
	switch(value)
	{
	case DescriptorType::e_SAMPLER: return "VK_DESCRIPTOR_TYPE_SAMPLER";
	case DescriptorType::e_COMBINED_IMAGE_SAMPLER: return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
	case DescriptorType::e_SAMPLED_IMAGE: return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
	case DescriptorType::e_STORAGE_IMAGE: return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
	case DescriptorType::e_UNIFORM_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
	case DescriptorType::e_STORAGE_TEXEL_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
	case DescriptorType::e_UNIFORM_BUFFER: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
	case DescriptorType::e_STORAGE_BUFFER: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
	case DescriptorType::e_UNIFORM_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
	case DescriptorType::e_STORAGE_BUFFER_DYNAMIC: return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
	case DescriptorType::e_INPUT_ATTACHMENT: return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
	case DescriptorType::e_INLINE_UNIFORM_BLOCK_EXT: return "VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT";
	case DescriptorType::e_ACCELERATION_STRUCTURE_NV: return "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV";
	default: return "invalid";
	}
}

enum class QueryType
{
	e_OCCLUSION = VK_QUERY_TYPE_OCCLUSION,
	e_PIPELINE_STATISTICS = VK_QUERY_TYPE_PIPELINE_STATISTICS,
	e_TIMESTAMP = VK_QUERY_TYPE_TIMESTAMP,
	e_TRANSFORM_FEEDBACK_STREAM_EXT = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT,
	e_PERFORMANCE_QUERY_KHR = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR,
	e_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV,
	e_PERFORMANCE_QUERY_INTEL = VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL,
	e_BEGIN_RANGE = VK_QUERY_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_QUERY_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_QUERY_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(QueryType)
inline std::string to_string(QueryType value)
{
	switch(value)
	{
	case QueryType::e_OCCLUSION: return "VK_QUERY_TYPE_OCCLUSION";
	case QueryType::e_PIPELINE_STATISTICS: return "VK_QUERY_TYPE_PIPELINE_STATISTICS";
	case QueryType::e_TIMESTAMP: return "VK_QUERY_TYPE_TIMESTAMP";
	case QueryType::e_TRANSFORM_FEEDBACK_STREAM_EXT: return "VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT";
	case QueryType::e_PERFORMANCE_QUERY_KHR: return "VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR";
	case QueryType::e_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV: return "VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV";
	case QueryType::e_PERFORMANCE_QUERY_INTEL: return "VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL";
	default: return "invalid";
	}
}

enum class BorderColor
{
	e_FLOAT_TRANSPARENT_BLACK = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
	e_INT_TRANSPARENT_BLACK = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
	e_FLOAT_OPAQUE_BLACK = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
	e_INT_OPAQUE_BLACK = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	e_FLOAT_OPAQUE_WHITE = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	e_INT_OPAQUE_WHITE = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
	e_BEGIN_RANGE = VK_BORDER_COLOR_BEGIN_RANGE,
	e_END_RANGE = VK_BORDER_COLOR_END_RANGE,
	e_RANGE_SIZE = VK_BORDER_COLOR_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(BorderColor)
inline std::string to_string(BorderColor value)
{
	switch(value)
	{
	case BorderColor::e_FLOAT_TRANSPARENT_BLACK: return "VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK";
	case BorderColor::e_INT_TRANSPARENT_BLACK: return "VK_BORDER_COLOR_INT_TRANSPARENT_BLACK";
	case BorderColor::e_FLOAT_OPAQUE_BLACK: return "VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK";
	case BorderColor::e_INT_OPAQUE_BLACK: return "VK_BORDER_COLOR_INT_OPAQUE_BLACK";
	case BorderColor::e_FLOAT_OPAQUE_WHITE: return "VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE";
	case BorderColor::e_INT_OPAQUE_WHITE: return "VK_BORDER_COLOR_INT_OPAQUE_WHITE";
	default: return "invalid";
	}
}

enum class PipelineBindPoint
{
	e_GRAPHICS = VK_PIPELINE_BIND_POINT_GRAPHICS,
	e_COMPUTE = VK_PIPELINE_BIND_POINT_COMPUTE,
	e_RAY_TRACING_NV = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,
	e_BEGIN_RANGE = VK_PIPELINE_BIND_POINT_BEGIN_RANGE,
	e_END_RANGE = VK_PIPELINE_BIND_POINT_END_RANGE,
	e_RANGE_SIZE = VK_PIPELINE_BIND_POINT_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PipelineBindPoint)
inline std::string to_string(PipelineBindPoint value)
{
	switch(value)
	{
	case PipelineBindPoint::e_GRAPHICS: return "VK_PIPELINE_BIND_POINT_GRAPHICS";
	case PipelineBindPoint::e_COMPUTE: return "VK_PIPELINE_BIND_POINT_COMPUTE";
	case PipelineBindPoint::e_RAY_TRACING_NV: return "VK_PIPELINE_BIND_POINT_RAY_TRACING_NV";
	default: return "invalid";
	}
}

enum class PipelineCacheHeaderVersion
{
	e_ONE = VK_PIPELINE_CACHE_HEADER_VERSION_ONE,
	e_BEGIN_RANGE = VK_PIPELINE_CACHE_HEADER_VERSION_BEGIN_RANGE,
	e_END_RANGE = VK_PIPELINE_CACHE_HEADER_VERSION_END_RANGE,
	e_RANGE_SIZE = VK_PIPELINE_CACHE_HEADER_VERSION_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PipelineCacheHeaderVersion)
inline std::string to_string(PipelineCacheHeaderVersion value)
{
	switch(value)
	{
	case PipelineCacheHeaderVersion::e_ONE: return "VK_PIPELINE_CACHE_HEADER_VERSION_ONE";
	default: return "invalid";
	}
}

enum class PrimitiveTopology
{
	e_POINT_LIST = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	e_LINE_LIST = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	e_LINE_STRIP = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	e_TRIANGLE_LIST = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	e_TRIANGLE_STRIP = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	e_TRIANGLE_FAN = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
	e_LINE_LIST_WITH_ADJACENCY = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
	e_LINE_STRIP_WITH_ADJACENCY = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
	e_TRIANGLE_LIST_WITH_ADJACENCY = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
	e_TRIANGLE_STRIP_WITH_ADJACENCY = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
	e_PATCH_LIST = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
	e_BEGIN_RANGE = VK_PRIMITIVE_TOPOLOGY_BEGIN_RANGE,
	e_END_RANGE = VK_PRIMITIVE_TOPOLOGY_END_RANGE,
	e_RANGE_SIZE = VK_PRIMITIVE_TOPOLOGY_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PrimitiveTopology)
inline std::string to_string(PrimitiveTopology value)
{
	switch(value)
	{
	case PrimitiveTopology::e_POINT_LIST: return "VK_PRIMITIVE_TOPOLOGY_POINT_LIST";
	case PrimitiveTopology::e_LINE_LIST: return "VK_PRIMITIVE_TOPOLOGY_LINE_LIST";
	case PrimitiveTopology::e_LINE_STRIP: return "VK_PRIMITIVE_TOPOLOGY_LINE_STRIP";
	case PrimitiveTopology::e_TRIANGLE_LIST: return "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST";
	case PrimitiveTopology::e_TRIANGLE_STRIP: return "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP";
	case PrimitiveTopology::e_TRIANGLE_FAN: return "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN";
	case PrimitiveTopology::e_LINE_LIST_WITH_ADJACENCY: return "VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY";
	case PrimitiveTopology::e_LINE_STRIP_WITH_ADJACENCY: return "VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY";
	case PrimitiveTopology::e_TRIANGLE_LIST_WITH_ADJACENCY: return "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY";
	case PrimitiveTopology::e_TRIANGLE_STRIP_WITH_ADJACENCY: return "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY";
	case PrimitiveTopology::e_PATCH_LIST: return "VK_PRIMITIVE_TOPOLOGY_PATCH_LIST";
	default: return "invalid";
	}
}

enum class SharingMode
{
	e_EXCLUSIVE = VK_SHARING_MODE_EXCLUSIVE,
	e_CONCURRENT = VK_SHARING_MODE_CONCURRENT,
	e_BEGIN_RANGE = VK_SHARING_MODE_BEGIN_RANGE,
	e_END_RANGE = VK_SHARING_MODE_END_RANGE,
	e_RANGE_SIZE = VK_SHARING_MODE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SharingMode)
inline std::string to_string(SharingMode value)
{
	switch(value)
	{
	case SharingMode::e_EXCLUSIVE: return "VK_SHARING_MODE_EXCLUSIVE";
	case SharingMode::e_CONCURRENT: return "VK_SHARING_MODE_CONCURRENT";
	default: return "invalid";
	}
}

enum class IndexType
{
	e_UINT16 = VK_INDEX_TYPE_UINT16,
	e_UINT32 = VK_INDEX_TYPE_UINT32,
	e_NONE_NV = VK_INDEX_TYPE_NONE_NV,
	e_UINT8_EXT = VK_INDEX_TYPE_UINT8_EXT,
	e_BEGIN_RANGE = VK_INDEX_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_INDEX_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_INDEX_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(IndexType)
inline std::string to_string(IndexType value)
{
	switch(value)
	{
	case IndexType::e_UINT16: return "VK_INDEX_TYPE_UINT16";
	case IndexType::e_UINT32: return "VK_INDEX_TYPE_UINT32";
	case IndexType::e_NONE_NV: return "VK_INDEX_TYPE_NONE_NV";
	case IndexType::e_UINT8_EXT: return "VK_INDEX_TYPE_UINT8_EXT";
	default: return "invalid";
	}
}

enum class Filter
{
	e_NEAREST = VK_FILTER_NEAREST,
	e_LINEAR = VK_FILTER_LINEAR,
	e_CUBIC_IMG = VK_FILTER_CUBIC_IMG,
	e_CUBIC_EXT = VK_FILTER_CUBIC_EXT,
	e_BEGIN_RANGE = VK_FILTER_BEGIN_RANGE,
	e_END_RANGE = VK_FILTER_END_RANGE,
	e_RANGE_SIZE = VK_FILTER_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(Filter)
inline std::string to_string(Filter value)
{
	switch(value)
	{
	case Filter::e_NEAREST: return "VK_FILTER_NEAREST";
	case Filter::e_LINEAR: return "VK_FILTER_LINEAR";
	case Filter::e_CUBIC_IMG: return "VK_FILTER_CUBIC_IMG";
	default: return "invalid";
	}
}

enum class SamplerMipmapMode
{
	e_NEAREST = VK_SAMPLER_MIPMAP_MODE_NEAREST,
	e_LINEAR = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	e_BEGIN_RANGE = VK_SAMPLER_MIPMAP_MODE_BEGIN_RANGE,
	e_END_RANGE = VK_SAMPLER_MIPMAP_MODE_END_RANGE,
	e_RANGE_SIZE = VK_SAMPLER_MIPMAP_MODE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SamplerMipmapMode)
inline std::string to_string(SamplerMipmapMode value)
{
	switch(value)
	{
	case SamplerMipmapMode::e_NEAREST: return "VK_SAMPLER_MIPMAP_MODE_NEAREST";
	case SamplerMipmapMode::e_LINEAR: return "VK_SAMPLER_MIPMAP_MODE_LINEAR";
	default: return "invalid";
	}
}

enum class SamplerAddressMode
{
	e_REPEAT = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	e_MIRRORED_REPEAT = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	e_CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	e_CLAMP_TO_BORDER = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	e_MIRROR_CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
	e_MIRROR_CLAMP_TO_EDGE_KHR = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE_KHR,
	e_BEGIN_RANGE = VK_SAMPLER_ADDRESS_MODE_BEGIN_RANGE,
	e_END_RANGE = VK_SAMPLER_ADDRESS_MODE_END_RANGE,
	e_RANGE_SIZE = VK_SAMPLER_ADDRESS_MODE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SamplerAddressMode)
inline std::string to_string(SamplerAddressMode value)
{
	switch(value)
	{
	case SamplerAddressMode::e_REPEAT: return "VK_SAMPLER_ADDRESS_MODE_REPEAT";
	case SamplerAddressMode::e_MIRRORED_REPEAT: return "VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT";
	case SamplerAddressMode::e_CLAMP_TO_EDGE: return "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE";
	case SamplerAddressMode::e_CLAMP_TO_BORDER: return "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER";
	case SamplerAddressMode::e_MIRROR_CLAMP_TO_EDGE: return "VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE";
	default: return "invalid";
	}
}

enum class CompareOp
{
	e_NEVER = VK_COMPARE_OP_NEVER,
	e_LESS = VK_COMPARE_OP_LESS,
	e_EQUAL = VK_COMPARE_OP_EQUAL,
	e_LESS_OR_EQUAL = VK_COMPARE_OP_LESS_OR_EQUAL,
	e_GREATER = VK_COMPARE_OP_GREATER,
	e_NOT_EQUAL = VK_COMPARE_OP_NOT_EQUAL,
	e_GREATER_OR_EQUAL = VK_COMPARE_OP_GREATER_OR_EQUAL,
	e_ALWAYS = VK_COMPARE_OP_ALWAYS,
	e_BEGIN_RANGE = VK_COMPARE_OP_BEGIN_RANGE,
	e_END_RANGE = VK_COMPARE_OP_END_RANGE,
	e_RANGE_SIZE = VK_COMPARE_OP_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CompareOp)
inline std::string to_string(CompareOp value)
{
	switch(value)
	{
	case CompareOp::e_NEVER: return "VK_COMPARE_OP_NEVER";
	case CompareOp::e_LESS: return "VK_COMPARE_OP_LESS";
	case CompareOp::e_EQUAL: return "VK_COMPARE_OP_EQUAL";
	case CompareOp::e_LESS_OR_EQUAL: return "VK_COMPARE_OP_LESS_OR_EQUAL";
	case CompareOp::e_GREATER: return "VK_COMPARE_OP_GREATER";
	case CompareOp::e_NOT_EQUAL: return "VK_COMPARE_OP_NOT_EQUAL";
	case CompareOp::e_GREATER_OR_EQUAL: return "VK_COMPARE_OP_GREATER_OR_EQUAL";
	case CompareOp::e_ALWAYS: return "VK_COMPARE_OP_ALWAYS";
	default: return "invalid";
	}
}

enum class PolygonMode
{
	e_FILL = VK_POLYGON_MODE_FILL,
	e_LINE = VK_POLYGON_MODE_LINE,
	e_POINT = VK_POLYGON_MODE_POINT,
	e_FILL_RECTANGLE_NV = VK_POLYGON_MODE_FILL_RECTANGLE_NV,
	e_BEGIN_RANGE = VK_POLYGON_MODE_BEGIN_RANGE,
	e_END_RANGE = VK_POLYGON_MODE_END_RANGE,
	e_RANGE_SIZE = VK_POLYGON_MODE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PolygonMode)
inline std::string to_string(PolygonMode value)
{
	switch(value)
	{
	case PolygonMode::e_FILL: return "VK_POLYGON_MODE_FILL";
	case PolygonMode::e_LINE: return "VK_POLYGON_MODE_LINE";
	case PolygonMode::e_POINT: return "VK_POLYGON_MODE_POINT";
	case PolygonMode::e_FILL_RECTANGLE_NV: return "VK_POLYGON_MODE_FILL_RECTANGLE_NV";
	default: return "invalid";
	}
}

enum class FrontFace
{
	e_COUNTER_CLOCKWISE = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	e_CLOCKWISE = VK_FRONT_FACE_CLOCKWISE,
	e_BEGIN_RANGE = VK_FRONT_FACE_BEGIN_RANGE,
	e_END_RANGE = VK_FRONT_FACE_END_RANGE,
	e_RANGE_SIZE = VK_FRONT_FACE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(FrontFace)
inline std::string to_string(FrontFace value)
{
	switch(value)
	{
	case FrontFace::e_COUNTER_CLOCKWISE: return "VK_FRONT_FACE_COUNTER_CLOCKWISE";
	case FrontFace::e_CLOCKWISE: return "VK_FRONT_FACE_CLOCKWISE";
	default: return "invalid";
	}
}

enum class BlendFactor
{
	e_ZERO = VK_BLEND_FACTOR_ZERO,
	e_ONE = VK_BLEND_FACTOR_ONE,
	e_SRC_COLOR = VK_BLEND_FACTOR_SRC_COLOR,
	e_ONE_MINUS_SRC_COLOR = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	e_DST_COLOR = VK_BLEND_FACTOR_DST_COLOR,
	e_ONE_MINUS_DST_COLOR = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	e_SRC_ALPHA = VK_BLEND_FACTOR_SRC_ALPHA,
	e_ONE_MINUS_SRC_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	e_DST_ALPHA = VK_BLEND_FACTOR_DST_ALPHA,
	e_ONE_MINUS_DST_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	e_CONSTANT_COLOR = VK_BLEND_FACTOR_CONSTANT_COLOR,
	e_ONE_MINUS_CONSTANT_COLOR = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	e_CONSTANT_ALPHA = VK_BLEND_FACTOR_CONSTANT_ALPHA,
	e_ONE_MINUS_CONSTANT_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
	e_SRC_ALPHA_SATURATE = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
	e_SRC1_COLOR = VK_BLEND_FACTOR_SRC1_COLOR,
	e_ONE_MINUS_SRC1_COLOR = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
	e_SRC1_ALPHA = VK_BLEND_FACTOR_SRC1_ALPHA,
	e_ONE_MINUS_SRC1_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
	e_BEGIN_RANGE = VK_BLEND_FACTOR_BEGIN_RANGE,
	e_END_RANGE = VK_BLEND_FACTOR_END_RANGE,
	e_RANGE_SIZE = VK_BLEND_FACTOR_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(BlendFactor)
inline std::string to_string(BlendFactor value)
{
	switch(value)
	{
	case BlendFactor::e_ZERO: return "VK_BLEND_FACTOR_ZERO";
	case BlendFactor::e_ONE: return "VK_BLEND_FACTOR_ONE";
	case BlendFactor::e_SRC_COLOR: return "VK_BLEND_FACTOR_SRC_COLOR";
	case BlendFactor::e_ONE_MINUS_SRC_COLOR: return "VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR";
	case BlendFactor::e_DST_COLOR: return "VK_BLEND_FACTOR_DST_COLOR";
	case BlendFactor::e_ONE_MINUS_DST_COLOR: return "VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR";
	case BlendFactor::e_SRC_ALPHA: return "VK_BLEND_FACTOR_SRC_ALPHA";
	case BlendFactor::e_ONE_MINUS_SRC_ALPHA: return "VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA";
	case BlendFactor::e_DST_ALPHA: return "VK_BLEND_FACTOR_DST_ALPHA";
	case BlendFactor::e_ONE_MINUS_DST_ALPHA: return "VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA";
	case BlendFactor::e_CONSTANT_COLOR: return "VK_BLEND_FACTOR_CONSTANT_COLOR";
	case BlendFactor::e_ONE_MINUS_CONSTANT_COLOR: return "VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR";
	case BlendFactor::e_CONSTANT_ALPHA: return "VK_BLEND_FACTOR_CONSTANT_ALPHA";
	case BlendFactor::e_ONE_MINUS_CONSTANT_ALPHA: return "VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA";
	case BlendFactor::e_SRC_ALPHA_SATURATE: return "VK_BLEND_FACTOR_SRC_ALPHA_SATURATE";
	case BlendFactor::e_SRC1_COLOR: return "VK_BLEND_FACTOR_SRC1_COLOR";
	case BlendFactor::e_ONE_MINUS_SRC1_COLOR: return "VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR";
	case BlendFactor::e_SRC1_ALPHA: return "VK_BLEND_FACTOR_SRC1_ALPHA";
	case BlendFactor::e_ONE_MINUS_SRC1_ALPHA: return "VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA";
	default: return "invalid";
	}
}

enum class BlendOp
{
	e_ADD = VK_BLEND_OP_ADD,
	e_SUBTRACT = VK_BLEND_OP_SUBTRACT,
	e_REVERSE_SUBTRACT = VK_BLEND_OP_REVERSE_SUBTRACT,
	e_MIN = VK_BLEND_OP_MIN,
	e_MAX = VK_BLEND_OP_MAX,
	e_ZERO_EXT = VK_BLEND_OP_ZERO_EXT,
	e_SRC_EXT = VK_BLEND_OP_SRC_EXT,
	e_DST_EXT = VK_BLEND_OP_DST_EXT,
	e_SRC_OVER_EXT = VK_BLEND_OP_SRC_OVER_EXT,
	e_DST_OVER_EXT = VK_BLEND_OP_DST_OVER_EXT,
	e_SRC_IN_EXT = VK_BLEND_OP_SRC_IN_EXT,
	e_DST_IN_EXT = VK_BLEND_OP_DST_IN_EXT,
	e_SRC_OUT_EXT = VK_BLEND_OP_SRC_OUT_EXT,
	e_DST_OUT_EXT = VK_BLEND_OP_DST_OUT_EXT,
	e_SRC_ATOP_EXT = VK_BLEND_OP_SRC_ATOP_EXT,
	e_DST_ATOP_EXT = VK_BLEND_OP_DST_ATOP_EXT,
	e_XOR_EXT = VK_BLEND_OP_XOR_EXT,
	e_MULTIPLY_EXT = VK_BLEND_OP_MULTIPLY_EXT,
	e_SCREEN_EXT = VK_BLEND_OP_SCREEN_EXT,
	e_OVERLAY_EXT = VK_BLEND_OP_OVERLAY_EXT,
	e_DARKEN_EXT = VK_BLEND_OP_DARKEN_EXT,
	e_LIGHTEN_EXT = VK_BLEND_OP_LIGHTEN_EXT,
	e_COLORDODGE_EXT = VK_BLEND_OP_COLORDODGE_EXT,
	e_COLORBURN_EXT = VK_BLEND_OP_COLORBURN_EXT,
	e_HARDLIGHT_EXT = VK_BLEND_OP_HARDLIGHT_EXT,
	e_SOFTLIGHT_EXT = VK_BLEND_OP_SOFTLIGHT_EXT,
	e_DIFFERENCE_EXT = VK_BLEND_OP_DIFFERENCE_EXT,
	e_EXCLUSION_EXT = VK_BLEND_OP_EXCLUSION_EXT,
	e_INVERT_EXT = VK_BLEND_OP_INVERT_EXT,
	e_INVERT_RGB_EXT = VK_BLEND_OP_INVERT_RGB_EXT,
	e_LINEARDODGE_EXT = VK_BLEND_OP_LINEARDODGE_EXT,
	e_LINEARBURN_EXT = VK_BLEND_OP_LINEARBURN_EXT,
	e_VIVIDLIGHT_EXT = VK_BLEND_OP_VIVIDLIGHT_EXT,
	e_LINEARLIGHT_EXT = VK_BLEND_OP_LINEARLIGHT_EXT,
	e_PINLIGHT_EXT = VK_BLEND_OP_PINLIGHT_EXT,
	e_HARDMIX_EXT = VK_BLEND_OP_HARDMIX_EXT,
	e_HSL_HUE_EXT = VK_BLEND_OP_HSL_HUE_EXT,
	e_HSL_SATURATION_EXT = VK_BLEND_OP_HSL_SATURATION_EXT,
	e_HSL_COLOR_EXT = VK_BLEND_OP_HSL_COLOR_EXT,
	e_HSL_LUMINOSITY_EXT = VK_BLEND_OP_HSL_LUMINOSITY_EXT,
	e_PLUS_EXT = VK_BLEND_OP_PLUS_EXT,
	e_PLUS_CLAMPED_EXT = VK_BLEND_OP_PLUS_CLAMPED_EXT,
	e_PLUS_CLAMPED_ALPHA_EXT = VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT,
	e_PLUS_DARKER_EXT = VK_BLEND_OP_PLUS_DARKER_EXT,
	e_MINUS_EXT = VK_BLEND_OP_MINUS_EXT,
	e_MINUS_CLAMPED_EXT = VK_BLEND_OP_MINUS_CLAMPED_EXT,
	e_CONTRAST_EXT = VK_BLEND_OP_CONTRAST_EXT,
	e_INVERT_OVG_EXT = VK_BLEND_OP_INVERT_OVG_EXT,
	e_RED_EXT = VK_BLEND_OP_RED_EXT,
	e_GREEN_EXT = VK_BLEND_OP_GREEN_EXT,
	e_BLUE_EXT = VK_BLEND_OP_BLUE_EXT,
	e_BEGIN_RANGE = VK_BLEND_OP_BEGIN_RANGE,
	e_END_RANGE = VK_BLEND_OP_END_RANGE,
	e_RANGE_SIZE = VK_BLEND_OP_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(BlendOp)
inline std::string to_string(BlendOp value)
{
	switch(value)
	{
	case BlendOp::e_ADD: return "VK_BLEND_OP_ADD";
	case BlendOp::e_SUBTRACT: return "VK_BLEND_OP_SUBTRACT";
	case BlendOp::e_REVERSE_SUBTRACT: return "VK_BLEND_OP_REVERSE_SUBTRACT";
	case BlendOp::e_MIN: return "VK_BLEND_OP_MIN";
	case BlendOp::e_MAX: return "VK_BLEND_OP_MAX";
	case BlendOp::e_ZERO_EXT: return "VK_BLEND_OP_ZERO_EXT";
	case BlendOp::e_SRC_EXT: return "VK_BLEND_OP_SRC_EXT";
	case BlendOp::e_DST_EXT: return "VK_BLEND_OP_DST_EXT";
	case BlendOp::e_SRC_OVER_EXT: return "VK_BLEND_OP_SRC_OVER_EXT";
	case BlendOp::e_DST_OVER_EXT: return "VK_BLEND_OP_DST_OVER_EXT";
	case BlendOp::e_SRC_IN_EXT: return "VK_BLEND_OP_SRC_IN_EXT";
	case BlendOp::e_DST_IN_EXT: return "VK_BLEND_OP_DST_IN_EXT";
	case BlendOp::e_SRC_OUT_EXT: return "VK_BLEND_OP_SRC_OUT_EXT";
	case BlendOp::e_DST_OUT_EXT: return "VK_BLEND_OP_DST_OUT_EXT";
	case BlendOp::e_SRC_ATOP_EXT: return "VK_BLEND_OP_SRC_ATOP_EXT";
	case BlendOp::e_DST_ATOP_EXT: return "VK_BLEND_OP_DST_ATOP_EXT";
	case BlendOp::e_XOR_EXT: return "VK_BLEND_OP_XOR_EXT";
	case BlendOp::e_MULTIPLY_EXT: return "VK_BLEND_OP_MULTIPLY_EXT";
	case BlendOp::e_SCREEN_EXT: return "VK_BLEND_OP_SCREEN_EXT";
	case BlendOp::e_OVERLAY_EXT: return "VK_BLEND_OP_OVERLAY_EXT";
	case BlendOp::e_DARKEN_EXT: return "VK_BLEND_OP_DARKEN_EXT";
	case BlendOp::e_LIGHTEN_EXT: return "VK_BLEND_OP_LIGHTEN_EXT";
	case BlendOp::e_COLORDODGE_EXT: return "VK_BLEND_OP_COLORDODGE_EXT";
	case BlendOp::e_COLORBURN_EXT: return "VK_BLEND_OP_COLORBURN_EXT";
	case BlendOp::e_HARDLIGHT_EXT: return "VK_BLEND_OP_HARDLIGHT_EXT";
	case BlendOp::e_SOFTLIGHT_EXT: return "VK_BLEND_OP_SOFTLIGHT_EXT";
	case BlendOp::e_DIFFERENCE_EXT: return "VK_BLEND_OP_DIFFERENCE_EXT";
	case BlendOp::e_EXCLUSION_EXT: return "VK_BLEND_OP_EXCLUSION_EXT";
	case BlendOp::e_INVERT_EXT: return "VK_BLEND_OP_INVERT_EXT";
	case BlendOp::e_INVERT_RGB_EXT: return "VK_BLEND_OP_INVERT_RGB_EXT";
	case BlendOp::e_LINEARDODGE_EXT: return "VK_BLEND_OP_LINEARDODGE_EXT";
	case BlendOp::e_LINEARBURN_EXT: return "VK_BLEND_OP_LINEARBURN_EXT";
	case BlendOp::e_VIVIDLIGHT_EXT: return "VK_BLEND_OP_VIVIDLIGHT_EXT";
	case BlendOp::e_LINEARLIGHT_EXT: return "VK_BLEND_OP_LINEARLIGHT_EXT";
	case BlendOp::e_PINLIGHT_EXT: return "VK_BLEND_OP_PINLIGHT_EXT";
	case BlendOp::e_HARDMIX_EXT: return "VK_BLEND_OP_HARDMIX_EXT";
	case BlendOp::e_HSL_HUE_EXT: return "VK_BLEND_OP_HSL_HUE_EXT";
	case BlendOp::e_HSL_SATURATION_EXT: return "VK_BLEND_OP_HSL_SATURATION_EXT";
	case BlendOp::e_HSL_COLOR_EXT: return "VK_BLEND_OP_HSL_COLOR_EXT";
	case BlendOp::e_HSL_LUMINOSITY_EXT: return "VK_BLEND_OP_HSL_LUMINOSITY_EXT";
	case BlendOp::e_PLUS_EXT: return "VK_BLEND_OP_PLUS_EXT";
	case BlendOp::e_PLUS_CLAMPED_EXT: return "VK_BLEND_OP_PLUS_CLAMPED_EXT";
	case BlendOp::e_PLUS_CLAMPED_ALPHA_EXT: return "VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT";
	case BlendOp::e_PLUS_DARKER_EXT: return "VK_BLEND_OP_PLUS_DARKER_EXT";
	case BlendOp::e_MINUS_EXT: return "VK_BLEND_OP_MINUS_EXT";
	case BlendOp::e_MINUS_CLAMPED_EXT: return "VK_BLEND_OP_MINUS_CLAMPED_EXT";
	case BlendOp::e_CONTRAST_EXT: return "VK_BLEND_OP_CONTRAST_EXT";
	case BlendOp::e_INVERT_OVG_EXT: return "VK_BLEND_OP_INVERT_OVG_EXT";
	case BlendOp::e_RED_EXT: return "VK_BLEND_OP_RED_EXT";
	case BlendOp::e_GREEN_EXT: return "VK_BLEND_OP_GREEN_EXT";
	case BlendOp::e_BLUE_EXT: return "VK_BLEND_OP_BLUE_EXT";
	default: return "invalid";
	}
}

enum class StencilOp
{
	e_KEEP = VK_STENCIL_OP_KEEP,
	e_ZERO = VK_STENCIL_OP_ZERO,
	e_REPLACE = VK_STENCIL_OP_REPLACE,
	e_INCREMENT_AND_CLAMP = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
	e_DECREMENT_AND_CLAMP = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
	e_INVERT = VK_STENCIL_OP_INVERT,
	e_INCREMENT_AND_WRAP = VK_STENCIL_OP_INCREMENT_AND_WRAP,
	e_DECREMENT_AND_WRAP = VK_STENCIL_OP_DECREMENT_AND_WRAP,
	e_BEGIN_RANGE = VK_STENCIL_OP_BEGIN_RANGE,
	e_END_RANGE = VK_STENCIL_OP_END_RANGE,
	e_RANGE_SIZE = VK_STENCIL_OP_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(StencilOp)
inline std::string to_string(StencilOp value)
{
	switch(value)
	{
	case StencilOp::e_KEEP: return "VK_STENCIL_OP_KEEP";
	case StencilOp::e_ZERO: return "VK_STENCIL_OP_ZERO";
	case StencilOp::e_REPLACE: return "VK_STENCIL_OP_REPLACE";
	case StencilOp::e_INCREMENT_AND_CLAMP: return "VK_STENCIL_OP_INCREMENT_AND_CLAMP";
	case StencilOp::e_DECREMENT_AND_CLAMP: return "VK_STENCIL_OP_DECREMENT_AND_CLAMP";
	case StencilOp::e_INVERT: return "VK_STENCIL_OP_INVERT";
	case StencilOp::e_INCREMENT_AND_WRAP: return "VK_STENCIL_OP_INCREMENT_AND_WRAP";
	case StencilOp::e_DECREMENT_AND_WRAP: return "VK_STENCIL_OP_DECREMENT_AND_WRAP";
	default: return "invalid";
	}
}

enum class LogicOp
{
	e_CLEAR = VK_LOGIC_OP_CLEAR,
	e_AND = VK_LOGIC_OP_AND,
	e_AND_REVERSE = VK_LOGIC_OP_AND_REVERSE,
	e_COPY = VK_LOGIC_OP_COPY,
	e_AND_INVERTED = VK_LOGIC_OP_AND_INVERTED,
	e_NO_OP = VK_LOGIC_OP_NO_OP,
	e_XOR = VK_LOGIC_OP_XOR,
	e_OR = VK_LOGIC_OP_OR,
	e_NOR = VK_LOGIC_OP_NOR,
	e_EQUIVALENT = VK_LOGIC_OP_EQUIVALENT,
	e_INVERT = VK_LOGIC_OP_INVERT,
	e_OR_REVERSE = VK_LOGIC_OP_OR_REVERSE,
	e_COPY_INVERTED = VK_LOGIC_OP_COPY_INVERTED,
	e_OR_INVERTED = VK_LOGIC_OP_OR_INVERTED,
	e_NAND = VK_LOGIC_OP_NAND,
	e_SET = VK_LOGIC_OP_SET,
	e_BEGIN_RANGE = VK_LOGIC_OP_BEGIN_RANGE,
	e_END_RANGE = VK_LOGIC_OP_END_RANGE,
	e_RANGE_SIZE = VK_LOGIC_OP_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(LogicOp)
inline std::string to_string(LogicOp value)
{
	switch(value)
	{
	case LogicOp::e_CLEAR: return "VK_LOGIC_OP_CLEAR";
	case LogicOp::e_AND: return "VK_LOGIC_OP_AND";
	case LogicOp::e_AND_REVERSE: return "VK_LOGIC_OP_AND_REVERSE";
	case LogicOp::e_COPY: return "VK_LOGIC_OP_COPY";
	case LogicOp::e_AND_INVERTED: return "VK_LOGIC_OP_AND_INVERTED";
	case LogicOp::e_NO_OP: return "VK_LOGIC_OP_NO_OP";
	case LogicOp::e_XOR: return "VK_LOGIC_OP_XOR";
	case LogicOp::e_OR: return "VK_LOGIC_OP_OR";
	case LogicOp::e_NOR: return "VK_LOGIC_OP_NOR";
	case LogicOp::e_EQUIVALENT: return "VK_LOGIC_OP_EQUIVALENT";
	case LogicOp::e_INVERT: return "VK_LOGIC_OP_INVERT";
	case LogicOp::e_OR_REVERSE: return "VK_LOGIC_OP_OR_REVERSE";
	case LogicOp::e_COPY_INVERTED: return "VK_LOGIC_OP_COPY_INVERTED";
	case LogicOp::e_OR_INVERTED: return "VK_LOGIC_OP_OR_INVERTED";
	case LogicOp::e_NAND: return "VK_LOGIC_OP_NAND";
	case LogicOp::e_SET: return "VK_LOGIC_OP_SET";
	default: return "invalid";
	}
}

enum class InternalAllocationType
{
	e_EXECUTABLE = VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE,
	e_BEGIN_RANGE = VK_INTERNAL_ALLOCATION_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_INTERNAL_ALLOCATION_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_INTERNAL_ALLOCATION_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(InternalAllocationType)
inline std::string to_string(InternalAllocationType value)
{
	switch(value)
	{
	case InternalAllocationType::e_EXECUTABLE: return "VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE";
	default: return "invalid";
	}
}

enum class SystemAllocationScope
{
	e_COMMAND = VK_SYSTEM_ALLOCATION_SCOPE_COMMAND,
	e_OBJECT = VK_SYSTEM_ALLOCATION_SCOPE_OBJECT,
	e_CACHE = VK_SYSTEM_ALLOCATION_SCOPE_CACHE,
	e_DEVICE = VK_SYSTEM_ALLOCATION_SCOPE_DEVICE,
	e_INSTANCE = VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE,
	e_BEGIN_RANGE = VK_SYSTEM_ALLOCATION_SCOPE_BEGIN_RANGE,
	e_END_RANGE = VK_SYSTEM_ALLOCATION_SCOPE_END_RANGE,
	e_RANGE_SIZE = VK_SYSTEM_ALLOCATION_SCOPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SystemAllocationScope)
inline std::string to_string(SystemAllocationScope value)
{
	switch(value)
	{
	case SystemAllocationScope::e_COMMAND: return "VK_SYSTEM_ALLOCATION_SCOPE_COMMAND";
	case SystemAllocationScope::e_OBJECT: return "VK_SYSTEM_ALLOCATION_SCOPE_OBJECT";
	case SystemAllocationScope::e_CACHE: return "VK_SYSTEM_ALLOCATION_SCOPE_CACHE";
	case SystemAllocationScope::e_DEVICE: return "VK_SYSTEM_ALLOCATION_SCOPE_DEVICE";
	case SystemAllocationScope::e_INSTANCE: return "VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE";
	default: return "invalid";
	}
}

enum class PhysicalDeviceType
{
	e_OTHER = VK_PHYSICAL_DEVICE_TYPE_OTHER,
	e_INTEGRATED_GPU = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
	e_DISCRETE_GPU = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
	e_VIRTUAL_GPU = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
	e_CPU = VK_PHYSICAL_DEVICE_TYPE_CPU,
	e_BEGIN_RANGE = VK_PHYSICAL_DEVICE_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_PHYSICAL_DEVICE_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PhysicalDeviceType)
inline std::string to_string(PhysicalDeviceType value)
{
	switch(value)
	{
	case PhysicalDeviceType::e_OTHER: return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
	case PhysicalDeviceType::e_INTEGRATED_GPU: return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
	case PhysicalDeviceType::e_DISCRETE_GPU: return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
	case PhysicalDeviceType::e_VIRTUAL_GPU: return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
	case PhysicalDeviceType::e_CPU: return "VK_PHYSICAL_DEVICE_TYPE_CPU";
	default: return "invalid";
	}
}

enum class VertexInputRate
{
	e_VERTEX = VK_VERTEX_INPUT_RATE_VERTEX,
	e_INSTANCE = VK_VERTEX_INPUT_RATE_INSTANCE,
	e_BEGIN_RANGE = VK_VERTEX_INPUT_RATE_BEGIN_RANGE,
	e_END_RANGE = VK_VERTEX_INPUT_RATE_END_RANGE,
	e_RANGE_SIZE = VK_VERTEX_INPUT_RATE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(VertexInputRate)
inline std::string to_string(VertexInputRate value)
{
	switch(value)
	{
	case VertexInputRate::e_VERTEX: return "VK_VERTEX_INPUT_RATE_VERTEX";
	case VertexInputRate::e_INSTANCE: return "VK_VERTEX_INPUT_RATE_INSTANCE";
	default: return "invalid";
	}
}

enum class Format
{
	e_UNDEFINED = VK_FORMAT_UNDEFINED,
	e_R4G4_UNORM_PACK8 = VK_FORMAT_R4G4_UNORM_PACK8,
	e_R4G4B4A4_UNORM_PACK16 = VK_FORMAT_R4G4B4A4_UNORM_PACK16,
	e_B4G4R4A4_UNORM_PACK16 = VK_FORMAT_B4G4R4A4_UNORM_PACK16,
	e_R5G6B5_UNORM_PACK16 = VK_FORMAT_R5G6B5_UNORM_PACK16,
	e_B5G6R5_UNORM_PACK16 = VK_FORMAT_B5G6R5_UNORM_PACK16,
	e_R5G5B5A1_UNORM_PACK16 = VK_FORMAT_R5G5B5A1_UNORM_PACK16,
	e_B5G5R5A1_UNORM_PACK16 = VK_FORMAT_B5G5R5A1_UNORM_PACK16,
	e_A1R5G5B5_UNORM_PACK16 = VK_FORMAT_A1R5G5B5_UNORM_PACK16,
	e_R8_UNORM = VK_FORMAT_R8_UNORM,
	e_R8_SNORM = VK_FORMAT_R8_SNORM,
	e_R8_USCALED = VK_FORMAT_R8_USCALED,
	e_R8_SSCALED = VK_FORMAT_R8_SSCALED,
	e_R8_UINT = VK_FORMAT_R8_UINT,
	e_R8_SINT = VK_FORMAT_R8_SINT,
	e_R8_SRGB = VK_FORMAT_R8_SRGB,
	e_R8G8_UNORM = VK_FORMAT_R8G8_UNORM,
	e_R8G8_SNORM = VK_FORMAT_R8G8_SNORM,
	e_R8G8_USCALED = VK_FORMAT_R8G8_USCALED,
	e_R8G8_SSCALED = VK_FORMAT_R8G8_SSCALED,
	e_R8G8_UINT = VK_FORMAT_R8G8_UINT,
	e_R8G8_SINT = VK_FORMAT_R8G8_SINT,
	e_R8G8_SRGB = VK_FORMAT_R8G8_SRGB,
	e_R8G8B8_UNORM = VK_FORMAT_R8G8B8_UNORM,
	e_R8G8B8_SNORM = VK_FORMAT_R8G8B8_SNORM,
	e_R8G8B8_USCALED = VK_FORMAT_R8G8B8_USCALED,
	e_R8G8B8_SSCALED = VK_FORMAT_R8G8B8_SSCALED,
	e_R8G8B8_UINT = VK_FORMAT_R8G8B8_UINT,
	e_R8G8B8_SINT = VK_FORMAT_R8G8B8_SINT,
	e_R8G8B8_SRGB = VK_FORMAT_R8G8B8_SRGB,
	e_B8G8R8_UNORM = VK_FORMAT_B8G8R8_UNORM,
	e_B8G8R8_SNORM = VK_FORMAT_B8G8R8_SNORM,
	e_B8G8R8_USCALED = VK_FORMAT_B8G8R8_USCALED,
	e_B8G8R8_SSCALED = VK_FORMAT_B8G8R8_SSCALED,
	e_B8G8R8_UINT = VK_FORMAT_B8G8R8_UINT,
	e_B8G8R8_SINT = VK_FORMAT_B8G8R8_SINT,
	e_B8G8R8_SRGB = VK_FORMAT_B8G8R8_SRGB,
	e_R8G8B8A8_UNORM = VK_FORMAT_R8G8B8A8_UNORM,
	e_R8G8B8A8_SNORM = VK_FORMAT_R8G8B8A8_SNORM,
	e_R8G8B8A8_USCALED = VK_FORMAT_R8G8B8A8_USCALED,
	e_R8G8B8A8_SSCALED = VK_FORMAT_R8G8B8A8_SSCALED,
	e_R8G8B8A8_UINT = VK_FORMAT_R8G8B8A8_UINT,
	e_R8G8B8A8_SINT = VK_FORMAT_R8G8B8A8_SINT,
	e_R8G8B8A8_SRGB = VK_FORMAT_R8G8B8A8_SRGB,
	e_B8G8R8A8_UNORM = VK_FORMAT_B8G8R8A8_UNORM,
	e_B8G8R8A8_SNORM = VK_FORMAT_B8G8R8A8_SNORM,
	e_B8G8R8A8_USCALED = VK_FORMAT_B8G8R8A8_USCALED,
	e_B8G8R8A8_SSCALED = VK_FORMAT_B8G8R8A8_SSCALED,
	e_B8G8R8A8_UINT = VK_FORMAT_B8G8R8A8_UINT,
	e_B8G8R8A8_SINT = VK_FORMAT_B8G8R8A8_SINT,
	e_B8G8R8A8_SRGB = VK_FORMAT_B8G8R8A8_SRGB,
	e_A8B8G8R8_UNORM_PACK32 = VK_FORMAT_A8B8G8R8_UNORM_PACK32,
	e_A8B8G8R8_SNORM_PACK32 = VK_FORMAT_A8B8G8R8_SNORM_PACK32,
	e_A8B8G8R8_USCALED_PACK32 = VK_FORMAT_A8B8G8R8_USCALED_PACK32,
	e_A8B8G8R8_SSCALED_PACK32 = VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
	e_A8B8G8R8_UINT_PACK32 = VK_FORMAT_A8B8G8R8_UINT_PACK32,
	e_A8B8G8R8_SINT_PACK32 = VK_FORMAT_A8B8G8R8_SINT_PACK32,
	e_A8B8G8R8_SRGB_PACK32 = VK_FORMAT_A8B8G8R8_SRGB_PACK32,
	e_A2R10G10B10_UNORM_PACK32 = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
	e_A2R10G10B10_SNORM_PACK32 = VK_FORMAT_A2R10G10B10_SNORM_PACK32,
	e_A2R10G10B10_USCALED_PACK32 = VK_FORMAT_A2R10G10B10_USCALED_PACK32,
	e_A2R10G10B10_SSCALED_PACK32 = VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
	e_A2R10G10B10_UINT_PACK32 = VK_FORMAT_A2R10G10B10_UINT_PACK32,
	e_A2R10G10B10_SINT_PACK32 = VK_FORMAT_A2R10G10B10_SINT_PACK32,
	e_A2B10G10R10_UNORM_PACK32 = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
	e_A2B10G10R10_SNORM_PACK32 = VK_FORMAT_A2B10G10R10_SNORM_PACK32,
	e_A2B10G10R10_USCALED_PACK32 = VK_FORMAT_A2B10G10R10_USCALED_PACK32,
	e_A2B10G10R10_SSCALED_PACK32 = VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
	e_A2B10G10R10_UINT_PACK32 = VK_FORMAT_A2B10G10R10_UINT_PACK32,
	e_A2B10G10R10_SINT_PACK32 = VK_FORMAT_A2B10G10R10_SINT_PACK32,
	e_R16_UNORM = VK_FORMAT_R16_UNORM,
	e_R16_SNORM = VK_FORMAT_R16_SNORM,
	e_R16_USCALED = VK_FORMAT_R16_USCALED,
	e_R16_SSCALED = VK_FORMAT_R16_SSCALED,
	e_R16_UINT = VK_FORMAT_R16_UINT,
	e_R16_SINT = VK_FORMAT_R16_SINT,
	e_R16_SFLOAT = VK_FORMAT_R16_SFLOAT,
	e_R16G16_UNORM = VK_FORMAT_R16G16_UNORM,
	e_R16G16_SNORM = VK_FORMAT_R16G16_SNORM,
	e_R16G16_USCALED = VK_FORMAT_R16G16_USCALED,
	e_R16G16_SSCALED = VK_FORMAT_R16G16_SSCALED,
	e_R16G16_UINT = VK_FORMAT_R16G16_UINT,
	e_R16G16_SINT = VK_FORMAT_R16G16_SINT,
	e_R16G16_SFLOAT = VK_FORMAT_R16G16_SFLOAT,
	e_R16G16B16_UNORM = VK_FORMAT_R16G16B16_UNORM,
	e_R16G16B16_SNORM = VK_FORMAT_R16G16B16_SNORM,
	e_R16G16B16_USCALED = VK_FORMAT_R16G16B16_USCALED,
	e_R16G16B16_SSCALED = VK_FORMAT_R16G16B16_SSCALED,
	e_R16G16B16_UINT = VK_FORMAT_R16G16B16_UINT,
	e_R16G16B16_SINT = VK_FORMAT_R16G16B16_SINT,
	e_R16G16B16_SFLOAT = VK_FORMAT_R16G16B16_SFLOAT,
	e_R16G16B16A16_UNORM = VK_FORMAT_R16G16B16A16_UNORM,
	e_R16G16B16A16_SNORM = VK_FORMAT_R16G16B16A16_SNORM,
	e_R16G16B16A16_USCALED = VK_FORMAT_R16G16B16A16_USCALED,
	e_R16G16B16A16_SSCALED = VK_FORMAT_R16G16B16A16_SSCALED,
	e_R16G16B16A16_UINT = VK_FORMAT_R16G16B16A16_UINT,
	e_R16G16B16A16_SINT = VK_FORMAT_R16G16B16A16_SINT,
	e_R16G16B16A16_SFLOAT = VK_FORMAT_R16G16B16A16_SFLOAT,
	e_R32_UINT = VK_FORMAT_R32_UINT,
	e_R32_SINT = VK_FORMAT_R32_SINT,
	e_R32_SFLOAT = VK_FORMAT_R32_SFLOAT,
	e_R32G32_UINT = VK_FORMAT_R32G32_UINT,
	e_R32G32_SINT = VK_FORMAT_R32G32_SINT,
	e_R32G32_SFLOAT = VK_FORMAT_R32G32_SFLOAT,
	e_R32G32B32_UINT = VK_FORMAT_R32G32B32_UINT,
	e_R32G32B32_SINT = VK_FORMAT_R32G32B32_SINT,
	e_R32G32B32_SFLOAT = VK_FORMAT_R32G32B32_SFLOAT,
	e_R32G32B32A32_UINT = VK_FORMAT_R32G32B32A32_UINT,
	e_R32G32B32A32_SINT = VK_FORMAT_R32G32B32A32_SINT,
	e_R32G32B32A32_SFLOAT = VK_FORMAT_R32G32B32A32_SFLOAT,
	e_R64_UINT = VK_FORMAT_R64_UINT,
	e_R64_SINT = VK_FORMAT_R64_SINT,
	e_R64_SFLOAT = VK_FORMAT_R64_SFLOAT,
	e_R64G64_UINT = VK_FORMAT_R64G64_UINT,
	e_R64G64_SINT = VK_FORMAT_R64G64_SINT,
	e_R64G64_SFLOAT = VK_FORMAT_R64G64_SFLOAT,
	e_R64G64B64_UINT = VK_FORMAT_R64G64B64_UINT,
	e_R64G64B64_SINT = VK_FORMAT_R64G64B64_SINT,
	e_R64G64B64_SFLOAT = VK_FORMAT_R64G64B64_SFLOAT,
	e_R64G64B64A64_UINT = VK_FORMAT_R64G64B64A64_UINT,
	e_R64G64B64A64_SINT = VK_FORMAT_R64G64B64A64_SINT,
	e_R64G64B64A64_SFLOAT = VK_FORMAT_R64G64B64A64_SFLOAT,
	e_B10G11R11_UFLOAT_PACK32 = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
	e_E5B9G9R9_UFLOAT_PACK32 = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
	e_D16_UNORM = VK_FORMAT_D16_UNORM,
	e_X8_D24_UNORM_PACK32 = VK_FORMAT_X8_D24_UNORM_PACK32,
	e_D32_SFLOAT = VK_FORMAT_D32_SFLOAT,
	e_S8_UINT = VK_FORMAT_S8_UINT,
	e_D16_UNORM_S8_UINT = VK_FORMAT_D16_UNORM_S8_UINT,
	e_D24_UNORM_S8_UINT = VK_FORMAT_D24_UNORM_S8_UINT,
	e_D32_SFLOAT_S8_UINT = VK_FORMAT_D32_SFLOAT_S8_UINT,
	e_BC1_RGB_UNORM_BLOCK = VK_FORMAT_BC1_RGB_UNORM_BLOCK,
	e_BC1_RGB_SRGB_BLOCK = VK_FORMAT_BC1_RGB_SRGB_BLOCK,
	e_BC1_RGBA_UNORM_BLOCK = VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	e_BC1_RGBA_SRGB_BLOCK = VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	e_BC2_UNORM_BLOCK = VK_FORMAT_BC2_UNORM_BLOCK,
	e_BC2_SRGB_BLOCK = VK_FORMAT_BC2_SRGB_BLOCK,
	e_BC3_UNORM_BLOCK = VK_FORMAT_BC3_UNORM_BLOCK,
	e_BC3_SRGB_BLOCK = VK_FORMAT_BC3_SRGB_BLOCK,
	e_BC4_UNORM_BLOCK = VK_FORMAT_BC4_UNORM_BLOCK,
	e_BC4_SNORM_BLOCK = VK_FORMAT_BC4_SNORM_BLOCK,
	e_BC5_UNORM_BLOCK = VK_FORMAT_BC5_UNORM_BLOCK,
	e_BC5_SNORM_BLOCK = VK_FORMAT_BC5_SNORM_BLOCK,
	e_BC6H_UFLOAT_BLOCK = VK_FORMAT_BC6H_UFLOAT_BLOCK,
	e_BC6H_SFLOAT_BLOCK = VK_FORMAT_BC6H_SFLOAT_BLOCK,
	e_BC7_UNORM_BLOCK = VK_FORMAT_BC7_UNORM_BLOCK,
	e_BC7_SRGB_BLOCK = VK_FORMAT_BC7_SRGB_BLOCK,
	e_ETC2_R8G8B8_UNORM_BLOCK = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
	e_ETC2_R8G8B8_SRGB_BLOCK = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
	e_ETC2_R8G8B8A1_UNORM_BLOCK = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
	e_ETC2_R8G8B8A1_SRGB_BLOCK = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
	e_ETC2_R8G8B8A8_UNORM_BLOCK = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
	e_ETC2_R8G8B8A8_SRGB_BLOCK = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
	e_EAC_R11_UNORM_BLOCK = VK_FORMAT_EAC_R11_UNORM_BLOCK,
	e_EAC_R11_SNORM_BLOCK = VK_FORMAT_EAC_R11_SNORM_BLOCK,
	e_EAC_R11G11_UNORM_BLOCK = VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
	e_EAC_R11G11_SNORM_BLOCK = VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
	e_ASTC_4x4_UNORM_BLOCK = VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
	e_ASTC_4x4_SRGB_BLOCK = VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
	e_ASTC_5x4_UNORM_BLOCK = VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
	e_ASTC_5x4_SRGB_BLOCK = VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
	e_ASTC_5x5_UNORM_BLOCK = VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
	e_ASTC_5x5_SRGB_BLOCK = VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
	e_ASTC_6x5_UNORM_BLOCK = VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
	e_ASTC_6x5_SRGB_BLOCK = VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
	e_ASTC_6x6_UNORM_BLOCK = VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
	e_ASTC_6x6_SRGB_BLOCK = VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
	e_ASTC_8x5_UNORM_BLOCK = VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
	e_ASTC_8x5_SRGB_BLOCK = VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
	e_ASTC_8x6_UNORM_BLOCK = VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
	e_ASTC_8x6_SRGB_BLOCK = VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
	e_ASTC_8x8_UNORM_BLOCK = VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
	e_ASTC_8x8_SRGB_BLOCK = VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
	e_ASTC_10x5_UNORM_BLOCK = VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
	e_ASTC_10x5_SRGB_BLOCK = VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
	e_ASTC_10x6_UNORM_BLOCK = VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
	e_ASTC_10x6_SRGB_BLOCK = VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
	e_ASTC_10x8_UNORM_BLOCK = VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
	e_ASTC_10x8_SRGB_BLOCK = VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
	e_ASTC_10x10_UNORM_BLOCK = VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
	e_ASTC_10x10_SRGB_BLOCK = VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
	e_ASTC_12x10_UNORM_BLOCK = VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
	e_ASTC_12x10_SRGB_BLOCK = VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
	e_ASTC_12x12_UNORM_BLOCK = VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
	e_ASTC_12x12_SRGB_BLOCK = VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
	e_PVRTC1_2BPP_UNORM_BLOCK_IMG = VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
	e_PVRTC1_4BPP_UNORM_BLOCK_IMG = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
	e_PVRTC2_2BPP_UNORM_BLOCK_IMG = VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
	e_PVRTC2_4BPP_UNORM_BLOCK_IMG = VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
	e_PVRTC1_2BPP_SRGB_BLOCK_IMG = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
	e_PVRTC1_4BPP_SRGB_BLOCK_IMG = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
	e_PVRTC2_2BPP_SRGB_BLOCK_IMG = VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
	e_PVRTC2_4BPP_SRGB_BLOCK_IMG = VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
	e_ASTC_4x4_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT,
	e_ASTC_5x4_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT,
	e_ASTC_5x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT,
	e_ASTC_6x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT,
	e_ASTC_6x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT,
	e_ASTC_8x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT,
	e_ASTC_8x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT,
	e_ASTC_8x8_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT,
	e_ASTC_10x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT,
	e_ASTC_10x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT,
	e_ASTC_10x8_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT,
	e_ASTC_10x10_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT,
	e_ASTC_12x10_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT,
	e_ASTC_12x12_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT,
	e_G8B8G8R8_422_UNORM = VK_FORMAT_G8B8G8R8_422_UNORM,
	e_B8G8R8G8_422_UNORM = VK_FORMAT_B8G8R8G8_422_UNORM,
	e_G8_B8_R8_3PLANE_420_UNORM = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
	e_G8_B8R8_2PLANE_420_UNORM = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
	e_G8_B8_R8_3PLANE_422_UNORM = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
	e_G8_B8R8_2PLANE_422_UNORM = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
	e_G8_B8_R8_3PLANE_444_UNORM = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
	e_R10X6_UNORM_PACK16 = VK_FORMAT_R10X6_UNORM_PACK16,
	e_R10X6G10X6_UNORM_2PACK16 = VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
	e_R10X6G10X6B10X6A10X6_UNORM_4PACK16 = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
	e_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
	e_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
	e_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
	e_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
	e_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
	e_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
	e_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
	e_R12X4_UNORM_PACK16 = VK_FORMAT_R12X4_UNORM_PACK16,
	e_R12X4G12X4_UNORM_2PACK16 = VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
	e_R12X4G12X4B12X4A12X4_UNORM_4PACK16 = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
	e_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
	e_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
	e_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
	e_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
	e_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
	e_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
	e_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
	e_G16B16G16R16_422_UNORM = VK_FORMAT_G16B16G16R16_422_UNORM,
	e_B16G16R16G16_422_UNORM = VK_FORMAT_B16G16R16G16_422_UNORM,
	e_G16_B16_R16_3PLANE_420_UNORM = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
	e_G16_B16R16_2PLANE_420_UNORM = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
	e_G16_B16_R16_3PLANE_422_UNORM = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
	e_G16_B16R16_2PLANE_422_UNORM = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
	e_G16_B16_R16_3PLANE_444_UNORM = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
	e_G8B8G8R8_422_UNORM_KHR = VK_FORMAT_G8B8G8R8_422_UNORM_KHR,
	e_B8G8R8G8_422_UNORM_KHR = VK_FORMAT_B8G8R8G8_422_UNORM_KHR,
	e_G8_B8_R8_3PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR,
	e_G8_B8R8_2PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
	e_G8_B8_R8_3PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR,
	e_G8_B8R8_2PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR,
	e_G8_B8_R8_3PLANE_444_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR,
	e_R10X6_UNORM_PACK16_KHR = VK_FORMAT_R10X6_UNORM_PACK16_KHR,
	e_R10X6G10X6_UNORM_2PACK16_KHR = VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR,
	e_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR,
	e_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR,
	e_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR,
	e_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR,
	e_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR,
	e_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR,
	e_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR,
	e_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR,
	e_R12X4_UNORM_PACK16_KHR = VK_FORMAT_R12X4_UNORM_PACK16_KHR,
	e_R12X4G12X4_UNORM_2PACK16_KHR = VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR,
	e_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR,
	e_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR,
	e_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR,
	e_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR,
	e_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR,
	e_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR,
	e_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR,
	e_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR,
	e_G16B16G16R16_422_UNORM_KHR = VK_FORMAT_G16B16G16R16_422_UNORM_KHR,
	e_B16G16R16G16_422_UNORM_KHR = VK_FORMAT_B16G16R16G16_422_UNORM_KHR,
	e_G16_B16_R16_3PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR,
	e_G16_B16R16_2PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR,
	e_G16_B16_R16_3PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR,
	e_G16_B16R16_2PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR,
	e_G16_B16_R16_3PLANE_444_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR,
	e_BEGIN_RANGE = VK_FORMAT_BEGIN_RANGE,
	e_END_RANGE = VK_FORMAT_END_RANGE,
	e_RANGE_SIZE = VK_FORMAT_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(Format)
inline std::string to_string(Format value)
{
	switch(value)
	{
	case Format::e_UNDEFINED: return "VK_FORMAT_UNDEFINED";
	case Format::e_R4G4_UNORM_PACK8: return "VK_FORMAT_R4G4_UNORM_PACK8";
	case Format::e_R4G4B4A4_UNORM_PACK16: return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
	case Format::e_B4G4R4A4_UNORM_PACK16: return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
	case Format::e_R5G6B5_UNORM_PACK16: return "VK_FORMAT_R5G6B5_UNORM_PACK16";
	case Format::e_B5G6R5_UNORM_PACK16: return "VK_FORMAT_B5G6R5_UNORM_PACK16";
	case Format::e_R5G5B5A1_UNORM_PACK16: return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
	case Format::e_B5G5R5A1_UNORM_PACK16: return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
	case Format::e_A1R5G5B5_UNORM_PACK16: return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
	case Format::e_R8_UNORM: return "VK_FORMAT_R8_UNORM";
	case Format::e_R8_SNORM: return "VK_FORMAT_R8_SNORM";
	case Format::e_R8_USCALED: return "VK_FORMAT_R8_USCALED";
	case Format::e_R8_SSCALED: return "VK_FORMAT_R8_SSCALED";
	case Format::e_R8_UINT: return "VK_FORMAT_R8_UINT";
	case Format::e_R8_SINT: return "VK_FORMAT_R8_SINT";
	case Format::e_R8_SRGB: return "VK_FORMAT_R8_SRGB";
	case Format::e_R8G8_UNORM: return "VK_FORMAT_R8G8_UNORM";
	case Format::e_R8G8_SNORM: return "VK_FORMAT_R8G8_SNORM";
	case Format::e_R8G8_USCALED: return "VK_FORMAT_R8G8_USCALED";
	case Format::e_R8G8_SSCALED: return "VK_FORMAT_R8G8_SSCALED";
	case Format::e_R8G8_UINT: return "VK_FORMAT_R8G8_UINT";
	case Format::e_R8G8_SINT: return "VK_FORMAT_R8G8_SINT";
	case Format::e_R8G8_SRGB: return "VK_FORMAT_R8G8_SRGB";
	case Format::e_R8G8B8_UNORM: return "VK_FORMAT_R8G8B8_UNORM";
	case Format::e_R8G8B8_SNORM: return "VK_FORMAT_R8G8B8_SNORM";
	case Format::e_R8G8B8_USCALED: return "VK_FORMAT_R8G8B8_USCALED";
	case Format::e_R8G8B8_SSCALED: return "VK_FORMAT_R8G8B8_SSCALED";
	case Format::e_R8G8B8_UINT: return "VK_FORMAT_R8G8B8_UINT";
	case Format::e_R8G8B8_SINT: return "VK_FORMAT_R8G8B8_SINT";
	case Format::e_R8G8B8_SRGB: return "VK_FORMAT_R8G8B8_SRGB";
	case Format::e_B8G8R8_UNORM: return "VK_FORMAT_B8G8R8_UNORM";
	case Format::e_B8G8R8_SNORM: return "VK_FORMAT_B8G8R8_SNORM";
	case Format::e_B8G8R8_USCALED: return "VK_FORMAT_B8G8R8_USCALED";
	case Format::e_B8G8R8_SSCALED: return "VK_FORMAT_B8G8R8_SSCALED";
	case Format::e_B8G8R8_UINT: return "VK_FORMAT_B8G8R8_UINT";
	case Format::e_B8G8R8_SINT: return "VK_FORMAT_B8G8R8_SINT";
	case Format::e_B8G8R8_SRGB: return "VK_FORMAT_B8G8R8_SRGB";
	case Format::e_R8G8B8A8_UNORM: return "VK_FORMAT_R8G8B8A8_UNORM";
	case Format::e_R8G8B8A8_SNORM: return "VK_FORMAT_R8G8B8A8_SNORM";
	case Format::e_R8G8B8A8_USCALED: return "VK_FORMAT_R8G8B8A8_USCALED";
	case Format::e_R8G8B8A8_SSCALED: return "VK_FORMAT_R8G8B8A8_SSCALED";
	case Format::e_R8G8B8A8_UINT: return "VK_FORMAT_R8G8B8A8_UINT";
	case Format::e_R8G8B8A8_SINT: return "VK_FORMAT_R8G8B8A8_SINT";
	case Format::e_R8G8B8A8_SRGB: return "VK_FORMAT_R8G8B8A8_SRGB";
	case Format::e_B8G8R8A8_UNORM: return "VK_FORMAT_B8G8R8A8_UNORM";
	case Format::e_B8G8R8A8_SNORM: return "VK_FORMAT_B8G8R8A8_SNORM";
	case Format::e_B8G8R8A8_USCALED: return "VK_FORMAT_B8G8R8A8_USCALED";
	case Format::e_B8G8R8A8_SSCALED: return "VK_FORMAT_B8G8R8A8_SSCALED";
	case Format::e_B8G8R8A8_UINT: return "VK_FORMAT_B8G8R8A8_UINT";
	case Format::e_B8G8R8A8_SINT: return "VK_FORMAT_B8G8R8A8_SINT";
	case Format::e_B8G8R8A8_SRGB: return "VK_FORMAT_B8G8R8A8_SRGB";
	case Format::e_A8B8G8R8_UNORM_PACK32: return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
	case Format::e_A8B8G8R8_SNORM_PACK32: return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
	case Format::e_A8B8G8R8_USCALED_PACK32: return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
	case Format::e_A8B8G8R8_SSCALED_PACK32: return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
	case Format::e_A8B8G8R8_UINT_PACK32: return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
	case Format::e_A8B8G8R8_SINT_PACK32: return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
	case Format::e_A8B8G8R8_SRGB_PACK32: return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
	case Format::e_A2R10G10B10_UNORM_PACK32: return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
	case Format::e_A2R10G10B10_SNORM_PACK32: return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
	case Format::e_A2R10G10B10_USCALED_PACK32: return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
	case Format::e_A2R10G10B10_SSCALED_PACK32: return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
	case Format::e_A2R10G10B10_UINT_PACK32: return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
	case Format::e_A2R10G10B10_SINT_PACK32: return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
	case Format::e_A2B10G10R10_UNORM_PACK32: return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
	case Format::e_A2B10G10R10_SNORM_PACK32: return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
	case Format::e_A2B10G10R10_USCALED_PACK32: return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
	case Format::e_A2B10G10R10_SSCALED_PACK32: return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
	case Format::e_A2B10G10R10_UINT_PACK32: return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
	case Format::e_A2B10G10R10_SINT_PACK32: return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
	case Format::e_R16_UNORM: return "VK_FORMAT_R16_UNORM";
	case Format::e_R16_SNORM: return "VK_FORMAT_R16_SNORM";
	case Format::e_R16_USCALED: return "VK_FORMAT_R16_USCALED";
	case Format::e_R16_SSCALED: return "VK_FORMAT_R16_SSCALED";
	case Format::e_R16_UINT: return "VK_FORMAT_R16_UINT";
	case Format::e_R16_SINT: return "VK_FORMAT_R16_SINT";
	case Format::e_R16_SFLOAT: return "VK_FORMAT_R16_SFLOAT";
	case Format::e_R16G16_UNORM: return "VK_FORMAT_R16G16_UNORM";
	case Format::e_R16G16_SNORM: return "VK_FORMAT_R16G16_SNORM";
	case Format::e_R16G16_USCALED: return "VK_FORMAT_R16G16_USCALED";
	case Format::e_R16G16_SSCALED: return "VK_FORMAT_R16G16_SSCALED";
	case Format::e_R16G16_UINT: return "VK_FORMAT_R16G16_UINT";
	case Format::e_R16G16_SINT: return "VK_FORMAT_R16G16_SINT";
	case Format::e_R16G16_SFLOAT: return "VK_FORMAT_R16G16_SFLOAT";
	case Format::e_R16G16B16_UNORM: return "VK_FORMAT_R16G16B16_UNORM";
	case Format::e_R16G16B16_SNORM: return "VK_FORMAT_R16G16B16_SNORM";
	case Format::e_R16G16B16_USCALED: return "VK_FORMAT_R16G16B16_USCALED";
	case Format::e_R16G16B16_SSCALED: return "VK_FORMAT_R16G16B16_SSCALED";
	case Format::e_R16G16B16_UINT: return "VK_FORMAT_R16G16B16_UINT";
	case Format::e_R16G16B16_SINT: return "VK_FORMAT_R16G16B16_SINT";
	case Format::e_R16G16B16_SFLOAT: return "VK_FORMAT_R16G16B16_SFLOAT";
	case Format::e_R16G16B16A16_UNORM: return "VK_FORMAT_R16G16B16A16_UNORM";
	case Format::e_R16G16B16A16_SNORM: return "VK_FORMAT_R16G16B16A16_SNORM";
	case Format::e_R16G16B16A16_USCALED: return "VK_FORMAT_R16G16B16A16_USCALED";
	case Format::e_R16G16B16A16_SSCALED: return "VK_FORMAT_R16G16B16A16_SSCALED";
	case Format::e_R16G16B16A16_UINT: return "VK_FORMAT_R16G16B16A16_UINT";
	case Format::e_R16G16B16A16_SINT: return "VK_FORMAT_R16G16B16A16_SINT";
	case Format::e_R16G16B16A16_SFLOAT: return "VK_FORMAT_R16G16B16A16_SFLOAT";
	case Format::e_R32_UINT: return "VK_FORMAT_R32_UINT";
	case Format::e_R32_SINT: return "VK_FORMAT_R32_SINT";
	case Format::e_R32_SFLOAT: return "VK_FORMAT_R32_SFLOAT";
	case Format::e_R32G32_UINT: return "VK_FORMAT_R32G32_UINT";
	case Format::e_R32G32_SINT: return "VK_FORMAT_R32G32_SINT";
	case Format::e_R32G32_SFLOAT: return "VK_FORMAT_R32G32_SFLOAT";
	case Format::e_R32G32B32_UINT: return "VK_FORMAT_R32G32B32_UINT";
	case Format::e_R32G32B32_SINT: return "VK_FORMAT_R32G32B32_SINT";
	case Format::e_R32G32B32_SFLOAT: return "VK_FORMAT_R32G32B32_SFLOAT";
	case Format::e_R32G32B32A32_UINT: return "VK_FORMAT_R32G32B32A32_UINT";
	case Format::e_R32G32B32A32_SINT: return "VK_FORMAT_R32G32B32A32_SINT";
	case Format::e_R32G32B32A32_SFLOAT: return "VK_FORMAT_R32G32B32A32_SFLOAT";
	case Format::e_R64_UINT: return "VK_FORMAT_R64_UINT";
	case Format::e_R64_SINT: return "VK_FORMAT_R64_SINT";
	case Format::e_R64_SFLOAT: return "VK_FORMAT_R64_SFLOAT";
	case Format::e_R64G64_UINT: return "VK_FORMAT_R64G64_UINT";
	case Format::e_R64G64_SINT: return "VK_FORMAT_R64G64_SINT";
	case Format::e_R64G64_SFLOAT: return "VK_FORMAT_R64G64_SFLOAT";
	case Format::e_R64G64B64_UINT: return "VK_FORMAT_R64G64B64_UINT";
	case Format::e_R64G64B64_SINT: return "VK_FORMAT_R64G64B64_SINT";
	case Format::e_R64G64B64_SFLOAT: return "VK_FORMAT_R64G64B64_SFLOAT";
	case Format::e_R64G64B64A64_UINT: return "VK_FORMAT_R64G64B64A64_UINT";
	case Format::e_R64G64B64A64_SINT: return "VK_FORMAT_R64G64B64A64_SINT";
	case Format::e_R64G64B64A64_SFLOAT: return "VK_FORMAT_R64G64B64A64_SFLOAT";
	case Format::e_B10G11R11_UFLOAT_PACK32: return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
	case Format::e_E5B9G9R9_UFLOAT_PACK32: return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
	case Format::e_D16_UNORM: return "VK_FORMAT_D16_UNORM";
	case Format::e_X8_D24_UNORM_PACK32: return "VK_FORMAT_X8_D24_UNORM_PACK32";
	case Format::e_D32_SFLOAT: return "VK_FORMAT_D32_SFLOAT";
	case Format::e_S8_UINT: return "VK_FORMAT_S8_UINT";
	case Format::e_D16_UNORM_S8_UINT: return "VK_FORMAT_D16_UNORM_S8_UINT";
	case Format::e_D24_UNORM_S8_UINT: return "VK_FORMAT_D24_UNORM_S8_UINT";
	case Format::e_D32_SFLOAT_S8_UINT: return "VK_FORMAT_D32_SFLOAT_S8_UINT";
	case Format::e_BC1_RGB_UNORM_BLOCK: return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
	case Format::e_BC1_RGB_SRGB_BLOCK: return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
	case Format::e_BC1_RGBA_UNORM_BLOCK: return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
	case Format::e_BC1_RGBA_SRGB_BLOCK: return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
	case Format::e_BC2_UNORM_BLOCK: return "VK_FORMAT_BC2_UNORM_BLOCK";
	case Format::e_BC2_SRGB_BLOCK: return "VK_FORMAT_BC2_SRGB_BLOCK";
	case Format::e_BC3_UNORM_BLOCK: return "VK_FORMAT_BC3_UNORM_BLOCK";
	case Format::e_BC3_SRGB_BLOCK: return "VK_FORMAT_BC3_SRGB_BLOCK";
	case Format::e_BC4_UNORM_BLOCK: return "VK_FORMAT_BC4_UNORM_BLOCK";
	case Format::e_BC4_SNORM_BLOCK: return "VK_FORMAT_BC4_SNORM_BLOCK";
	case Format::e_BC5_UNORM_BLOCK: return "VK_FORMAT_BC5_UNORM_BLOCK";
	case Format::e_BC5_SNORM_BLOCK: return "VK_FORMAT_BC5_SNORM_BLOCK";
	case Format::e_BC6H_UFLOAT_BLOCK: return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
	case Format::e_BC6H_SFLOAT_BLOCK: return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
	case Format::e_BC7_UNORM_BLOCK: return "VK_FORMAT_BC7_UNORM_BLOCK";
	case Format::e_BC7_SRGB_BLOCK: return "VK_FORMAT_BC7_SRGB_BLOCK";
	case Format::e_ETC2_R8G8B8_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
	case Format::e_ETC2_R8G8B8_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
	case Format::e_ETC2_R8G8B8A1_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
	case Format::e_ETC2_R8G8B8A1_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
	case Format::e_ETC2_R8G8B8A8_UNORM_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
	case Format::e_ETC2_R8G8B8A8_SRGB_BLOCK: return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
	case Format::e_EAC_R11_UNORM_BLOCK: return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
	case Format::e_EAC_R11_SNORM_BLOCK: return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
	case Format::e_EAC_R11G11_UNORM_BLOCK: return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
	case Format::e_EAC_R11G11_SNORM_BLOCK: return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
	case Format::e_ASTC_4x4_UNORM_BLOCK: return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
	case Format::e_ASTC_4x4_SRGB_BLOCK: return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
	case Format::e_ASTC_5x4_UNORM_BLOCK: return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
	case Format::e_ASTC_5x4_SRGB_BLOCK: return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
	case Format::e_ASTC_5x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
	case Format::e_ASTC_5x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
	case Format::e_ASTC_6x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
	case Format::e_ASTC_6x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
	case Format::e_ASTC_6x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
	case Format::e_ASTC_6x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
	case Format::e_ASTC_8x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
	case Format::e_ASTC_8x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
	case Format::e_ASTC_8x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
	case Format::e_ASTC_8x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
	case Format::e_ASTC_8x8_UNORM_BLOCK: return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
	case Format::e_ASTC_8x8_SRGB_BLOCK: return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
	case Format::e_ASTC_10x5_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
	case Format::e_ASTC_10x5_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
	case Format::e_ASTC_10x6_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
	case Format::e_ASTC_10x6_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
	case Format::e_ASTC_10x8_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
	case Format::e_ASTC_10x8_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
	case Format::e_ASTC_10x10_UNORM_BLOCK: return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
	case Format::e_ASTC_10x10_SRGB_BLOCK: return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
	case Format::e_ASTC_12x10_UNORM_BLOCK: return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
	case Format::e_ASTC_12x10_SRGB_BLOCK: return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
	case Format::e_ASTC_12x12_UNORM_BLOCK: return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
	case Format::e_ASTC_12x12_SRGB_BLOCK: return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
	case Format::e_PVRTC1_2BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG";
	case Format::e_PVRTC1_4BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG";
	case Format::e_PVRTC2_2BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG";
	case Format::e_PVRTC2_4BPP_UNORM_BLOCK_IMG: return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG";
	case Format::e_PVRTC1_2BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG";
	case Format::e_PVRTC1_4BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG";
	case Format::e_PVRTC2_2BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG";
	case Format::e_PVRTC2_4BPP_SRGB_BLOCK_IMG: return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG";
	case Format::e_ASTC_4x4_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_5x4_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_5x5_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_6x5_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_6x6_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_8x5_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_8x6_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_8x8_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_10x5_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_10x6_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_10x8_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_10x10_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_12x10_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT";
	case Format::e_ASTC_12x12_SFLOAT_BLOCK_EXT: return "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT";
	case Format::e_G8B8G8R8_422_UNORM: return "VK_FORMAT_G8B8G8R8_422_UNORM";
	case Format::e_B8G8R8G8_422_UNORM: return "VK_FORMAT_B8G8R8G8_422_UNORM";
	case Format::e_G8_B8_R8_3PLANE_420_UNORM: return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM";
	case Format::e_G8_B8R8_2PLANE_420_UNORM: return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM";
	case Format::e_G8_B8_R8_3PLANE_422_UNORM: return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM";
	case Format::e_G8_B8R8_2PLANE_422_UNORM: return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM";
	case Format::e_G8_B8_R8_3PLANE_444_UNORM: return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM";
	case Format::e_R10X6_UNORM_PACK16: return "VK_FORMAT_R10X6_UNORM_PACK16";
	case Format::e_R10X6G10X6_UNORM_2PACK16: return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16";
	case Format::e_R10X6G10X6B10X6A10X6_UNORM_4PACK16: return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16";
	case Format::e_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
	case Format::e_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
	case Format::e_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16: return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
	case Format::e_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
	case Format::e_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16: return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
	case Format::e_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16: return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
	case Format::e_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
	case Format::e_R12X4_UNORM_PACK16: return "VK_FORMAT_R12X4_UNORM_PACK16";
	case Format::e_R12X4G12X4_UNORM_2PACK16: return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16";
	case Format::e_R12X4G12X4B12X4A12X4_UNORM_4PACK16: return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16";
	case Format::e_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
	case Format::e_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
	case Format::e_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16: return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
	case Format::e_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16: return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
	case Format::e_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16: return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
	case Format::e_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
	case Format::e_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
	case Format::e_G16B16G16R16_422_UNORM: return "VK_FORMAT_G16B16G16R16_422_UNORM";
	case Format::e_B16G16R16G16_422_UNORM: return "VK_FORMAT_B16G16R16G16_422_UNORM";
	case Format::e_G16_B16_R16_3PLANE_420_UNORM: return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM";
	case Format::e_G16_B16R16_2PLANE_420_UNORM: return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM";
	case Format::e_G16_B16_R16_3PLANE_422_UNORM: return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM";
	case Format::e_G16_B16R16_2PLANE_422_UNORM: return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM";
	case Format::e_G16_B16_R16_3PLANE_444_UNORM: return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM";
	default: return "invalid";
	}
}

enum class StructureType
{
	e_APPLICATION_INFO = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	e_INSTANCE_CREATE_INFO = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	e_DEVICE_QUEUE_CREATE_INFO = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
	e_DEVICE_CREATE_INFO = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	e_SUBMIT_INFO = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	e_MEMORY_ALLOCATE_INFO = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	e_MAPPED_MEMORY_RANGE = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
	e_BIND_SPARSE_INFO = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,
	e_FENCE_CREATE_INFO = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	e_SEMAPHORE_CREATE_INFO = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	e_EVENT_CREATE_INFO = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
	e_QUERY_POOL_CREATE_INFO = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
	e_BUFFER_CREATE_INFO = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	e_BUFFER_VIEW_CREATE_INFO = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
	e_IMAGE_CREATE_INFO = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	e_IMAGE_VIEW_CREATE_INFO = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	e_SHADER_MODULE_CREATE_INFO = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	e_PIPELINE_CACHE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
	e_PIPELINE_SHADER_STAGE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	e_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	e_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	e_PIPELINE_TESSELLATION_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
	e_PIPELINE_VIEWPORT_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	e_PIPELINE_RASTERIZATION_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	e_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	e_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	e_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	e_PIPELINE_DYNAMIC_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	e_GRAPHICS_PIPELINE_CREATE_INFO = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	e_COMPUTE_PIPELINE_CREATE_INFO = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
	e_PIPELINE_LAYOUT_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	e_SAMPLER_CREATE_INFO = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
	e_DESCRIPTOR_SET_LAYOUT_CREATE_INFO = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	e_DESCRIPTOR_POOL_CREATE_INFO = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	e_DESCRIPTOR_SET_ALLOCATE_INFO = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	e_WRITE_DESCRIPTOR_SET = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	e_COPY_DESCRIPTOR_SET = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
	e_FRAMEBUFFER_CREATE_INFO = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
	e_RENDER_PASS_CREATE_INFO = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	e_COMMAND_POOL_CREATE_INFO = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	e_COMMAND_BUFFER_ALLOCATE_INFO = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	e_COMMAND_BUFFER_INHERITANCE_INFO = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
	e_COMMAND_BUFFER_BEGIN_INFO = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	e_RENDER_PASS_BEGIN_INFO = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	e_BUFFER_MEMORY_BARRIER = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
	e_IMAGE_MEMORY_BARRIER = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	e_MEMORY_BARRIER = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
	e_LOADER_INSTANCE_CREATE_INFO = VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO,
	e_LOADER_DEVICE_CREATE_INFO = VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO,
	e_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
	e_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
	e_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
	e_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES,
	e_SWAPCHAIN_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	e_PRESENT_INFO_KHR = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	e_DISPLAY_MODE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR,
	e_DISPLAY_SURFACE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
	e_DISPLAY_PRESENT_INFO_KHR = VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR,
	e_XLIB_SURFACE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
	e_XCB_SURFACE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
	e_WAYLAND_SURFACE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
	e_ANDROID_SURFACE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
	e_WIN32_SURFACE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
	e_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
	e_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD,
	e_DEBUG_MARKER_OBJECT_NAME_INFO_EXT = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
	e_DEBUG_MARKER_OBJECT_TAG_INFO_EXT = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT,
	e_DEBUG_MARKER_MARKER_INFO_EXT = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
	e_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV,
	e_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV,
	e_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV,
	e_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT,
	e_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT,
	e_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT,
	e_IMAGE_VIEW_HANDLE_INFO_NVX = VK_STRUCTURE_TYPE_IMAGE_VIEW_HANDLE_INFO_NVX,
	e_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD = VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD,
	e_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP = VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP,
	e_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV,
	e_RENDER_PASS_MULTIVIEW_CREATE_INFO = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
	e_PHYSICAL_DEVICE_MULTIVIEW_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES,
	e_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES,
	e_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV,
	e_EXPORT_MEMORY_ALLOCATE_INFO_NV = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV,
	e_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV,
	e_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV = VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV,
	e_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV = VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV,
	e_PHYSICAL_DEVICE_FEATURES_2 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
	e_PHYSICAL_DEVICE_PROPERTIES_2 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
	e_FORMAT_PROPERTIES_2 = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2,
	e_IMAGE_FORMAT_PROPERTIES_2 = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2,
	e_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
	e_QUEUE_FAMILY_PROPERTIES_2 = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2,
	e_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
	e_SPARSE_IMAGE_FORMAT_PROPERTIES_2 = VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2,
	e_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2,
	e_MEMORY_ALLOCATE_FLAGS_INFO = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
	e_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO,
	e_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO,
	e_DEVICE_GROUP_SUBMIT_INFO = VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO,
	e_DEVICE_GROUP_BIND_SPARSE_INFO = VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO,
	e_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR = VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR,
	e_IMAGE_SWAPCHAIN_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR,
	e_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR,
	e_ACQUIRE_NEXT_IMAGE_INFO_KHR = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
	e_DEVICE_GROUP_PRESENT_INFO_KHR = VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR,
	e_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR,
	e_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO,
	e_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO,
	e_VALIDATION_FLAGS_EXT = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT,
	e_VI_SURFACE_CREATE_INFO_NN = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN,
	e_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
	e_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT,
	e_IMAGE_VIEW_ASTC_DECODE_MODE_EXT = VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT,
	e_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT,
	e_PHYSICAL_DEVICE_GROUP_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES,
	e_DEVICE_GROUP_DEVICE_CREATE_INFO = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO,
	e_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO,
	e_EXTERNAL_IMAGE_FORMAT_PROPERTIES = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES,
	e_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO,
	e_EXTERNAL_BUFFER_PROPERTIES = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES,
	e_PHYSICAL_DEVICE_ID_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES,
	e_EXTERNAL_MEMORY_BUFFER_CREATE_INFO = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO,
	e_EXTERNAL_MEMORY_IMAGE_CREATE_INFO = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
	e_EXPORT_MEMORY_ALLOCATE_INFO = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
	e_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
	e_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR = VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
	e_MEMORY_WIN32_HANDLE_PROPERTIES_KHR = VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR,
	e_MEMORY_GET_WIN32_HANDLE_INFO_KHR = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR,
	e_IMPORT_MEMORY_FD_INFO_KHR = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR,
	e_MEMORY_FD_PROPERTIES_KHR = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR,
	e_MEMORY_GET_FD_INFO_KHR = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
	e_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR = VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR,
	e_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO,
	e_EXTERNAL_SEMAPHORE_PROPERTIES = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES,
	e_EXPORT_SEMAPHORE_CREATE_INFO = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO,
	e_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR,
	e_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR,
	e_D3D12_FENCE_SUBMIT_INFO_KHR = VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR,
	e_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR,
	e_IMPORT_SEMAPHORE_FD_INFO_KHR = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR,
	e_SEMAPHORE_GET_FD_INFO_KHR = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR,
	e_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR,
	e_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT,
	e_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT,
	e_CONDITIONAL_RENDERING_BEGIN_INFO_EXT = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT,
	e_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES,
	e_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES,
	e_PRESENT_REGIONS_KHR = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR,
	e_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
	e_OBJECT_TABLE_CREATE_INFO_NVX = VK_STRUCTURE_TYPE_OBJECT_TABLE_CREATE_INFO_NVX,
	e_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NVX = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NVX,
	e_CMD_PROCESS_COMMANDS_INFO_NVX = VK_STRUCTURE_TYPE_CMD_PROCESS_COMMANDS_INFO_NVX,
	e_CMD_RESERVE_SPACE_FOR_COMMANDS_INFO_NVX = VK_STRUCTURE_TYPE_CMD_RESERVE_SPACE_FOR_COMMANDS_INFO_NVX,
	e_DEVICE_GENERATED_COMMANDS_LIMITS_NVX = VK_STRUCTURE_TYPE_DEVICE_GENERATED_COMMANDS_LIMITS_NVX,
	e_DEVICE_GENERATED_COMMANDS_FEATURES_NVX = VK_STRUCTURE_TYPE_DEVICE_GENERATED_COMMANDS_FEATURES_NVX,
	e_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV,
	e_SURFACE_CAPABILITIES_2_EXT = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT,
	e_DISPLAY_POWER_INFO_EXT = VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT,
	e_DEVICE_EVENT_INFO_EXT = VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT,
	e_DISPLAY_EVENT_INFO_EXT = VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT,
	e_SWAPCHAIN_COUNTER_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT,
	e_PRESENT_TIMES_INFO_GOOGLE = VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE,
	e_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES,
	e_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX,
	e_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV,
	e_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT,
	e_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT,
	e_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT,
	e_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT,
	e_HDR_METADATA_EXT = VK_STRUCTURE_TYPE_HDR_METADATA_EXT,
	e_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES,
	e_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO,
	e_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO,
	e_RENDER_PASS_ATTACHMENT_BEGIN_INFO = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
	e_ATTACHMENT_DESCRIPTION_2 = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
	e_ATTACHMENT_REFERENCE_2 = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
	e_SUBPASS_DESCRIPTION_2 = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
	e_SUBPASS_DEPENDENCY_2 = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
	e_RENDER_PASS_CREATE_INFO_2 = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
	e_SUBPASS_BEGIN_INFO = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO,
	e_SUBPASS_END_INFO = VK_STRUCTURE_TYPE_SUBPASS_END_INFO,
	e_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR = VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR,
	e_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO,
	e_EXTERNAL_FENCE_PROPERTIES = VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES,
	e_EXPORT_FENCE_CREATE_INFO = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO,
	e_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR = VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR,
	e_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR = VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR,
	e_FENCE_GET_WIN32_HANDLE_INFO_KHR = VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR,
	e_IMPORT_FENCE_FD_INFO_KHR = VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR,
	e_FENCE_GET_FD_INFO_KHR = VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR,
	e_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR,
	e_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR,
	e_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR,
	e_PERFORMANCE_QUERY_SUBMIT_INFO_KHR = VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR,
	e_ACQUIRE_PROFILING_LOCK_INFO_KHR = VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR,
	e_PERFORMANCE_COUNTER_KHR = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR,
	e_PERFORMANCE_COUNTER_DESCRIPTION_KHR = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR,
	e_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES,
	e_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO = VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO,
	e_IMAGE_VIEW_USAGE_CREATE_INFO = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO,
	e_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO,
	e_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
	e_SURFACE_CAPABILITIES_2_KHR = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
	e_SURFACE_FORMAT_2_KHR = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
	e_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES,
	e_DISPLAY_PROPERTIES_2_KHR = VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR,
	e_DISPLAY_PLANE_PROPERTIES_2_KHR = VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR,
	e_DISPLAY_MODE_PROPERTIES_2_KHR = VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR,
	e_DISPLAY_PLANE_INFO_2_KHR = VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR,
	e_DISPLAY_PLANE_CAPABILITIES_2_KHR = VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR,
	e_IOS_SURFACE_CREATE_INFO_MVK = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK,
	e_MACOS_SURFACE_CREATE_INFO_MVK = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK,
	e_MEMORY_DEDICATED_REQUIREMENTS = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS,
	e_MEMORY_DEDICATED_ALLOCATE_INFO = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
	e_DEBUG_UTILS_OBJECT_NAME_INFO_EXT = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
	e_DEBUG_UTILS_OBJECT_TAG_INFO_EXT = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT,
	e_DEBUG_UTILS_LABEL_EXT = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
	e_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT,
	e_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	e_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID,
	e_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
	e_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID,
	e_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID,
	e_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID = VK_STRUCTURE_TYPE_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID,
	e_EXTERNAL_FORMAT_ANDROID = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
	e_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES,
	e_SAMPLER_REDUCTION_MODE_CREATE_INFO = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO,
	e_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT,
	e_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES_EXT,
	e_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT,
	e_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO_EXT,
	e_SAMPLE_LOCATIONS_INFO_EXT = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT,
	e_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT = VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT,
	e_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT,
	e_MULTISAMPLE_PROPERTIES_EXT = VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT,
	e_PROTECTED_SUBMIT_INFO = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO,
	e_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES,
	e_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES,
	e_DEVICE_QUEUE_INFO_2 = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
	e_BUFFER_MEMORY_REQUIREMENTS_INFO_2 = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
	e_IMAGE_MEMORY_REQUIREMENTS_INFO_2 = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
	e_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2 = VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2,
	e_MEMORY_REQUIREMENTS_2 = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
	e_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2 = VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2,
	e_IMAGE_FORMAT_LIST_CREATE_INFO = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO,
	e_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT,
	e_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT,
	e_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT,
	e_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV,
	e_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV,
	e_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV,
	e_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV,
	e_SAMPLER_YCBCR_CONVERSION_CREATE_INFO = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO,
	e_SAMPLER_YCBCR_CONVERSION_INFO = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO,
	e_BIND_IMAGE_PLANE_MEMORY_INFO = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO,
	e_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO = VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO,
	e_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES,
	e_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES,
	e_BIND_BUFFER_MEMORY_INFO = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
	e_BIND_IMAGE_MEMORY_INFO = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
	e_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT,
	e_DRM_FORMAT_MODIFIER_PROPERTIES_EXT = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT,
	e_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT,
	e_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT,
	e_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT,
	e_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT,
	e_VALIDATION_CACHE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT,
	e_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT,
	e_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
	e_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
	e_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES,
	e_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
	e_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT,
	e_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV,
	e_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV,
	e_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV,
	e_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV,
	e_RAY_TRACING_PIPELINE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV,
	e_ACCELERATION_STRUCTURE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV,
	e_GEOMETRY_NV = VK_STRUCTURE_TYPE_GEOMETRY_NV,
	e_GEOMETRY_TRIANGLES_NV = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV,
	e_GEOMETRY_AABB_NV = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV,
	e_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV,
	e_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV,
	e_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV,
	e_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV,
	e_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV,
	e_ACCELERATION_STRUCTURE_INFO_NV = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV,
	e_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV,
	e_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV,
	e_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES,
	e_DESCRIPTOR_SET_LAYOUT_SUPPORT = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT,
	e_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT,
	e_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT = VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT,
	e_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES,
	e_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES,
	e_IMPORT_MEMORY_HOST_POINTER_INFO_EXT = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT,
	e_MEMORY_HOST_POINTER_PROPERTIES_EXT = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT,
	e_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT,
	e_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES,
	e_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR,
	e_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD = VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD,
	e_CALIBRATED_TIMESTAMP_INFO_EXT = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT,
	e_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD,
	e_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD = VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD,
	e_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT,
	e_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT,
	e_PRESENT_FRAME_TOKEN_GGP = VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP,
	e_PIPELINE_CREATION_FEEDBACK_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_DRIVER_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
	e_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES,
	e_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES,
	e_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE,
	e_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV,
	e_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV,
	e_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV,
	e_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_NV,
	e_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV,
	e_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV,
	e_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV,
	e_CHECKPOINT_DATA_NV = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV,
	e_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV = VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV,
	e_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
	e_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES,
	e_SEMAPHORE_TYPE_CREATE_INFO = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
	e_TIMELINE_SEMAPHORE_SUBMIT_INFO = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
	e_SEMAPHORE_WAIT_INFO = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
	e_SEMAPHORE_SIGNAL_INFO = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
	e_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL,
	e_QUERY_POOL_CREATE_INFO_INTEL = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO_INTEL,
	e_INITIALIZE_PERFORMANCE_API_INFO_INTEL = VK_STRUCTURE_TYPE_INITIALIZE_PERFORMANCE_API_INFO_INTEL,
	e_PERFORMANCE_MARKER_INFO_INTEL = VK_STRUCTURE_TYPE_PERFORMANCE_MARKER_INFO_INTEL,
	e_PERFORMANCE_STREAM_MARKER_INFO_INTEL = VK_STRUCTURE_TYPE_PERFORMANCE_STREAM_MARKER_INFO_INTEL,
	e_PERFORMANCE_OVERRIDE_INFO_INTEL = VK_STRUCTURE_TYPE_PERFORMANCE_OVERRIDE_INFO_INTEL,
	e_PERFORMANCE_CONFIGURATION_ACQUIRE_INFO_INTEL = VK_STRUCTURE_TYPE_PERFORMANCE_CONFIGURATION_ACQUIRE_INFO_INTEL,
	e_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES,
	e_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT,
	e_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD = VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD,
	e_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD = VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD,
	e_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA = VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA,
	e_METAL_SURFACE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT,
	e_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT,
	e_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
	e_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT,
	e_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT,
	e_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD,
	e_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD,
	e_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT,
	e_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT,
	e_MEMORY_PRIORITY_ALLOCATE_INFO_EXT = VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT,
	e_SURFACE_PROTECTED_CAPABILITIES_KHR = VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR,
	e_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV,
	e_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES,
	e_ATTACHMENT_REFERENCE_STENCIL_LAYOUT = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT,
	e_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT,
	e_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT,
	e_BUFFER_DEVICE_ADDRESS_INFO = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
	e_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT,
	e_IMAGE_STENCIL_USAGE_CREATE_INFO = VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO,
	e_VALIDATION_FEATURES_EXT = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
	e_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV,
	e_COOPERATIVE_MATRIX_PROPERTIES_NV = VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_NV,
	e_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV,
	e_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV,
	e_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV = VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV,
	e_FRAMEBUFFER_MIXED_SAMPLES_COMBINATION_NV = VK_STRUCTURE_TYPE_FRAMEBUFFER_MIXED_SAMPLES_COMBINATION_NV,
	e_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT,
	e_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT,
	e_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES,
	e_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT,
	e_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT,
	e_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT,
	e_HEADLESS_SURFACE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
	e_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO = VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO,
	e_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO = VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO,
	e_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO = VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO,
	e_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT,
	e_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT,
	e_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES,
	e_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT,
	e_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR,
	e_PIPELINE_INFO_KHR = VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR,
	e_PIPELINE_EXECUTABLE_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR,
	e_PIPELINE_EXECUTABLE_INFO_KHR = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR,
	e_PIPELINE_EXECUTABLE_STATISTIC_KHR = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR,
	e_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR,
	e_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT,
	e_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT,
	e_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES_EXT,
	e_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES,
	e_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES,
	e_DEBUG_REPORT_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
	e_RENDER_PASS_MULTIVIEW_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO_KHR,
	e_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR,
	e_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES_KHR,
	e_PHYSICAL_DEVICE_FEATURES_2_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR,
	e_PHYSICAL_DEVICE_PROPERTIES_2_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR,
	e_FORMAT_PROPERTIES_2_KHR = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2_KHR,
	e_IMAGE_FORMAT_PROPERTIES_2_KHR = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2_KHR,
	e_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR,
	e_QUEUE_FAMILY_PROPERTIES_2_KHR = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR,
	e_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR,
	e_SPARSE_IMAGE_FORMAT_PROPERTIES_2_KHR = VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2_KHR,
	e_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2_KHR,
	e_MEMORY_ALLOCATE_FLAGS_INFO_KHR = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR,
	e_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO_KHR = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO_KHR,
	e_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO_KHR = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO_KHR,
	e_DEVICE_GROUP_SUBMIT_INFO_KHR = VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO_KHR,
	e_DEVICE_GROUP_BIND_SPARSE_INFO_KHR = VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO_KHR,
	e_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO_KHR = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO_KHR,
	e_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO_KHR = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO_KHR,
	e_PHYSICAL_DEVICE_GROUP_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES_KHR,
	e_DEVICE_GROUP_DEVICE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO_KHR,
	e_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO_KHR,
	e_EXTERNAL_IMAGE_FORMAT_PROPERTIES_KHR = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES_KHR,
	e_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO_KHR,
	e_EXTERNAL_BUFFER_PROPERTIES_KHR = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES_KHR,
	e_PHYSICAL_DEVICE_ID_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES_KHR,
	e_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR,
	e_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR,
	e_EXPORT_MEMORY_ALLOCATE_INFO_KHR = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR,
	e_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR,
	e_EXTERNAL_SEMAPHORE_PROPERTIES_KHR = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR,
	e_EXPORT_SEMAPHORE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR,
	e_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR,
	e_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR,
	e_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR,
	e_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO_KHR,
	e_SURFACE_CAPABILITIES2_EXT = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES2_EXT,
	e_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR,
	e_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR,
	e_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR,
	e_RENDER_PASS_ATTACHMENT_BEGIN_INFO_KHR = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO_KHR,
	e_ATTACHMENT_DESCRIPTION_2_KHR = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR,
	e_ATTACHMENT_REFERENCE_2_KHR = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR,
	e_SUBPASS_DESCRIPTION_2_KHR = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR,
	e_SUBPASS_DEPENDENCY_2_KHR = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR,
	e_RENDER_PASS_CREATE_INFO_2_KHR = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR,
	e_SUBPASS_BEGIN_INFO_KHR = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR,
	e_SUBPASS_END_INFO_KHR = VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR,
	e_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO_KHR,
	e_EXTERNAL_FENCE_PROPERTIES_KHR = VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES_KHR,
	e_EXPORT_FENCE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO_KHR,
	e_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES_KHR,
	e_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO_KHR,
	e_IMAGE_VIEW_USAGE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO_KHR,
	e_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO_KHR,
	e_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR,
	e_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES_KHR,
	e_MEMORY_DEDICATED_REQUIREMENTS_KHR = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR,
	e_MEMORY_DEDICATED_ALLOCATE_INFO_KHR = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR,
	e_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES_EXT,
	e_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT,
	e_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR,
	e_IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR,
	e_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2_KHR = VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2_KHR,
	e_MEMORY_REQUIREMENTS_2_KHR = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR,
	e_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2_KHR = VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2_KHR,
	e_IMAGE_FORMAT_LIST_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO_KHR,
	e_SAMPLER_YCBCR_CONVERSION_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO_KHR,
	e_SAMPLER_YCBCR_CONVERSION_INFO_KHR = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO_KHR,
	e_BIND_IMAGE_PLANE_MEMORY_INFO_KHR = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO_KHR,
	e_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO_KHR = VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO_KHR,
	e_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES_KHR,
	e_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES_KHR = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES_KHR,
	e_BIND_BUFFER_MEMORY_INFO_KHR = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO_KHR,
	e_BIND_IMAGE_MEMORY_INFO_KHR = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO_KHR,
	e_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
	e_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT,
	e_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
	e_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT_EXT = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT_EXT,
	e_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES_KHR,
	e_DESCRIPTOR_SET_LAYOUT_SUPPORT_KHR = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT_KHR,
	e_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES_KHR,
	e_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR,
	e_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR,
	e_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR,
	e_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES_KHR,
	e_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES_KHR,
	e_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR,
	e_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR,
	e_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES_KHR,
	e_SEMAPHORE_TYPE_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR,
	e_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR,
	e_SEMAPHORE_WAIT_INFO_KHR = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO_KHR,
	e_SEMAPHORE_SIGNAL_INFO_KHR = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO_KHR,
	e_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES_KHR,
	e_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT,
	e_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES_KHR,
	e_ATTACHMENT_REFERENCE_STENCIL_LAYOUT_KHR = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT_KHR,
	e_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT_KHR = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT_KHR,
	e_PHYSICAL_DEVICE_BUFFER_ADDRESS_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_ADDRESS_FEATURES_EXT,
	e_BUFFER_DEVICE_ADDRESS_INFO_EXT = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT,
	e_IMAGE_STENCIL_USAGE_CREATE_INFO_EXT = VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO_EXT,
	e_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES_KHR,
	e_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR,
	e_BUFFER_DEVICE_ADDRESS_INFO_KHR = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
	e_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO_KHR = VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO_KHR,
	e_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO_KHR = VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO_KHR,
	e_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO_KHR = VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO_KHR,
	e_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT,
	e_BEGIN_RANGE = VK_STRUCTURE_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_STRUCTURE_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_STRUCTURE_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(StructureType)
inline std::string to_string(StructureType value)
{
	switch(value)
	{
	case StructureType::e_APPLICATION_INFO: return "VK_STRUCTURE_TYPE_APPLICATION_INFO";
	case StructureType::e_INSTANCE_CREATE_INFO: return "VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO";
	case StructureType::e_DEVICE_QUEUE_CREATE_INFO: return "VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO";
	case StructureType::e_DEVICE_CREATE_INFO: return "VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO";
	case StructureType::e_SUBMIT_INFO: return "VK_STRUCTURE_TYPE_SUBMIT_INFO";
	case StructureType::e_MEMORY_ALLOCATE_INFO: return "VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO";
	case StructureType::e_MAPPED_MEMORY_RANGE: return "VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE";
	case StructureType::e_BIND_SPARSE_INFO: return "VK_STRUCTURE_TYPE_BIND_SPARSE_INFO";
	case StructureType::e_FENCE_CREATE_INFO: return "VK_STRUCTURE_TYPE_FENCE_CREATE_INFO";
	case StructureType::e_SEMAPHORE_CREATE_INFO: return "VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO";
	case StructureType::e_EVENT_CREATE_INFO: return "VK_STRUCTURE_TYPE_EVENT_CREATE_INFO";
	case StructureType::e_QUERY_POOL_CREATE_INFO: return "VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO";
	case StructureType::e_BUFFER_CREATE_INFO: return "VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO";
	case StructureType::e_BUFFER_VIEW_CREATE_INFO: return "VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO";
	case StructureType::e_IMAGE_CREATE_INFO: return "VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO";
	case StructureType::e_IMAGE_VIEW_CREATE_INFO: return "VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO";
	case StructureType::e_SHADER_MODULE_CREATE_INFO: return "VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO";
	case StructureType::e_PIPELINE_CACHE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO";
	case StructureType::e_PIPELINE_SHADER_STAGE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO";
	case StructureType::e_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO";
	case StructureType::e_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO";
	case StructureType::e_PIPELINE_TESSELLATION_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO";
	case StructureType::e_PIPELINE_VIEWPORT_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO";
	case StructureType::e_PIPELINE_RASTERIZATION_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO";
	case StructureType::e_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO";
	case StructureType::e_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO";
	case StructureType::e_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO";
	case StructureType::e_PIPELINE_DYNAMIC_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO";
	case StructureType::e_GRAPHICS_PIPELINE_CREATE_INFO: return "VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO";
	case StructureType::e_COMPUTE_PIPELINE_CREATE_INFO: return "VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO";
	case StructureType::e_PIPELINE_LAYOUT_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO";
	case StructureType::e_SAMPLER_CREATE_INFO: return "VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO";
	case StructureType::e_DESCRIPTOR_SET_LAYOUT_CREATE_INFO: return "VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO";
	case StructureType::e_DESCRIPTOR_POOL_CREATE_INFO: return "VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO";
	case StructureType::e_DESCRIPTOR_SET_ALLOCATE_INFO: return "VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO";
	case StructureType::e_WRITE_DESCRIPTOR_SET: return "VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET";
	case StructureType::e_COPY_DESCRIPTOR_SET: return "VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET";
	case StructureType::e_FRAMEBUFFER_CREATE_INFO: return "VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO";
	case StructureType::e_RENDER_PASS_CREATE_INFO: return "VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO";
	case StructureType::e_COMMAND_POOL_CREATE_INFO: return "VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO";
	case StructureType::e_COMMAND_BUFFER_ALLOCATE_INFO: return "VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO";
	case StructureType::e_COMMAND_BUFFER_INHERITANCE_INFO: return "VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO";
	case StructureType::e_COMMAND_BUFFER_BEGIN_INFO: return "VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO";
	case StructureType::e_RENDER_PASS_BEGIN_INFO: return "VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO";
	case StructureType::e_BUFFER_MEMORY_BARRIER: return "VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER";
	case StructureType::e_IMAGE_MEMORY_BARRIER: return "VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER";
	case StructureType::e_MEMORY_BARRIER: return "VK_STRUCTURE_TYPE_MEMORY_BARRIER";
	case StructureType::e_LOADER_INSTANCE_CREATE_INFO: return "VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO";
	case StructureType::e_LOADER_DEVICE_CREATE_INFO: return "VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO";
	case StructureType::e_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES";
	case StructureType::e_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES";
	case StructureType::e_SWAPCHAIN_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR";
	case StructureType::e_PRESENT_INFO_KHR: return "VK_STRUCTURE_TYPE_PRESENT_INFO_KHR";
	case StructureType::e_DISPLAY_MODE_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR";
	case StructureType::e_DISPLAY_SURFACE_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR";
	case StructureType::e_DISPLAY_PRESENT_INFO_KHR: return "VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR";
	case StructureType::e_XLIB_SURFACE_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR";
	case StructureType::e_XCB_SURFACE_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR";
	case StructureType::e_WAYLAND_SURFACE_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR";
	case StructureType::e_ANDROID_SURFACE_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR";
	case StructureType::e_WIN32_SURFACE_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR";
	case StructureType::e_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT";
	case StructureType::e_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD: return "VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD";
	case StructureType::e_DEBUG_MARKER_OBJECT_NAME_INFO_EXT: return "VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT";
	case StructureType::e_DEBUG_MARKER_OBJECT_TAG_INFO_EXT: return "VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT";
	case StructureType::e_DEBUG_MARKER_MARKER_INFO_EXT: return "VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT";
	case StructureType::e_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV";
	case StructureType::e_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV";
	case StructureType::e_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV: return "VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV";
	case StructureType::e_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT";
	case StructureType::e_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT";
	case StructureType::e_IMAGE_VIEW_HANDLE_INFO_NVX: return "VK_STRUCTURE_TYPE_IMAGE_VIEW_HANDLE_INFO_NVX";
	case StructureType::e_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD: return "VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD";
	case StructureType::e_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP: return "VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP";
	case StructureType::e_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV";
	case StructureType::e_RENDER_PASS_MULTIVIEW_CREATE_INFO: return "VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO";
	case StructureType::e_PHYSICAL_DEVICE_MULTIVIEW_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES";
	case StructureType::e_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV";
	case StructureType::e_EXPORT_MEMORY_ALLOCATE_INFO_NV: return "VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV";
	case StructureType::e_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV: return "VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV";
	case StructureType::e_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV: return "VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV";
	case StructureType::e_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV: return "VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV";
	case StructureType::e_PHYSICAL_DEVICE_FEATURES_2: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2";
	case StructureType::e_PHYSICAL_DEVICE_PROPERTIES_2: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2";
	case StructureType::e_FORMAT_PROPERTIES_2: return "VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2";
	case StructureType::e_IMAGE_FORMAT_PROPERTIES_2: return "VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2";
	case StructureType::e_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2";
	case StructureType::e_QUEUE_FAMILY_PROPERTIES_2: return "VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2";
	case StructureType::e_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2";
	case StructureType::e_SPARSE_IMAGE_FORMAT_PROPERTIES_2: return "VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2";
	case StructureType::e_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2";
	case StructureType::e_MEMORY_ALLOCATE_FLAGS_INFO: return "VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO";
	case StructureType::e_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO: return "VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO";
	case StructureType::e_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO: return "VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO";
	case StructureType::e_DEVICE_GROUP_SUBMIT_INFO: return "VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO";
	case StructureType::e_DEVICE_GROUP_BIND_SPARSE_INFO: return "VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO";
	case StructureType::e_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR: return "VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR";
	case StructureType::e_IMAGE_SWAPCHAIN_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR";
	case StructureType::e_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR: return "VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR";
	case StructureType::e_ACQUIRE_NEXT_IMAGE_INFO_KHR: return "VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR";
	case StructureType::e_DEVICE_GROUP_PRESENT_INFO_KHR: return "VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR";
	case StructureType::e_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR";
	case StructureType::e_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO: return "VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO";
	case StructureType::e_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO: return "VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO";
	case StructureType::e_VALIDATION_FLAGS_EXT: return "VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT";
	case StructureType::e_VI_SURFACE_CREATE_INFO_NN: return "VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT";
	case StructureType::e_IMAGE_VIEW_ASTC_DECODE_MODE_EXT: return "VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT";
	case StructureType::e_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_GROUP_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES";
	case StructureType::e_DEVICE_GROUP_DEVICE_CREATE_INFO: return "VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO";
	case StructureType::e_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO";
	case StructureType::e_EXTERNAL_IMAGE_FORMAT_PROPERTIES: return "VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES";
	case StructureType::e_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO";
	case StructureType::e_EXTERNAL_BUFFER_PROPERTIES: return "VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES";
	case StructureType::e_PHYSICAL_DEVICE_ID_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES";
	case StructureType::e_EXTERNAL_MEMORY_BUFFER_CREATE_INFO: return "VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO";
	case StructureType::e_EXTERNAL_MEMORY_IMAGE_CREATE_INFO: return "VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO";
	case StructureType::e_EXPORT_MEMORY_ALLOCATE_INFO: return "VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO";
	case StructureType::e_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR: return "VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR";
	case StructureType::e_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR: return "VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR";
	case StructureType::e_MEMORY_WIN32_HANDLE_PROPERTIES_KHR: return "VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR";
	case StructureType::e_MEMORY_GET_WIN32_HANDLE_INFO_KHR: return "VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR";
	case StructureType::e_IMPORT_MEMORY_FD_INFO_KHR: return "VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR";
	case StructureType::e_MEMORY_FD_PROPERTIES_KHR: return "VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR";
	case StructureType::e_MEMORY_GET_FD_INFO_KHR: return "VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR";
	case StructureType::e_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR: return "VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR";
	case StructureType::e_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO";
	case StructureType::e_EXTERNAL_SEMAPHORE_PROPERTIES: return "VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES";
	case StructureType::e_EXPORT_SEMAPHORE_CREATE_INFO: return "VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO";
	case StructureType::e_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR: return "VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR";
	case StructureType::e_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR: return "VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR";
	case StructureType::e_D3D12_FENCE_SUBMIT_INFO_KHR: return "VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR";
	case StructureType::e_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR: return "VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR";
	case StructureType::e_IMPORT_SEMAPHORE_FD_INFO_KHR: return "VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR";
	case StructureType::e_SEMAPHORE_GET_FD_INFO_KHR: return "VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR";
	case StructureType::e_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR";
	case StructureType::e_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT: return "VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT";
	case StructureType::e_CONDITIONAL_RENDERING_BEGIN_INFO_EXT: return "VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES";
	case StructureType::e_PRESENT_REGIONS_KHR: return "VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR";
	case StructureType::e_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO";
	case StructureType::e_OBJECT_TABLE_CREATE_INFO_NVX: return "VK_STRUCTURE_TYPE_OBJECT_TABLE_CREATE_INFO_NVX";
	case StructureType::e_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NVX: return "VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NVX";
	case StructureType::e_CMD_PROCESS_COMMANDS_INFO_NVX: return "VK_STRUCTURE_TYPE_CMD_PROCESS_COMMANDS_INFO_NVX";
	case StructureType::e_CMD_RESERVE_SPACE_FOR_COMMANDS_INFO_NVX: return "VK_STRUCTURE_TYPE_CMD_RESERVE_SPACE_FOR_COMMANDS_INFO_NVX";
	case StructureType::e_DEVICE_GENERATED_COMMANDS_LIMITS_NVX: return "VK_STRUCTURE_TYPE_DEVICE_GENERATED_COMMANDS_LIMITS_NVX";
	case StructureType::e_DEVICE_GENERATED_COMMANDS_FEATURES_NVX: return "VK_STRUCTURE_TYPE_DEVICE_GENERATED_COMMANDS_FEATURES_NVX";
	case StructureType::e_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV";
	case StructureType::e_SURFACE_CAPABILITIES_2_EXT: return "VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT";
	case StructureType::e_DISPLAY_POWER_INFO_EXT: return "VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT";
	case StructureType::e_DEVICE_EVENT_INFO_EXT: return "VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT";
	case StructureType::e_DISPLAY_EVENT_INFO_EXT: return "VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT";
	case StructureType::e_SWAPCHAIN_COUNTER_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT";
	case StructureType::e_PRESENT_TIMES_INFO_GOOGLE: return "VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE";
	case StructureType::e_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES";
	case StructureType::e_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX";
	case StructureType::e_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV";
	case StructureType::e_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT";
	case StructureType::e_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT";
	case StructureType::e_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT";
	case StructureType::e_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT";
	case StructureType::e_HDR_METADATA_EXT: return "VK_STRUCTURE_TYPE_HDR_METADATA_EXT";
	case StructureType::e_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES";
	case StructureType::e_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO: return "VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO";
	case StructureType::e_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO: return "VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO";
	case StructureType::e_RENDER_PASS_ATTACHMENT_BEGIN_INFO: return "VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO";
	case StructureType::e_ATTACHMENT_DESCRIPTION_2: return "VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2";
	case StructureType::e_ATTACHMENT_REFERENCE_2: return "VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2";
	case StructureType::e_SUBPASS_DESCRIPTION_2: return "VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2";
	case StructureType::e_SUBPASS_DEPENDENCY_2: return "VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2";
	case StructureType::e_RENDER_PASS_CREATE_INFO_2: return "VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2";
	case StructureType::e_SUBPASS_BEGIN_INFO: return "VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO";
	case StructureType::e_SUBPASS_END_INFO: return "VK_STRUCTURE_TYPE_SUBPASS_END_INFO";
	case StructureType::e_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR: return "VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR";
	case StructureType::e_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO";
	case StructureType::e_EXTERNAL_FENCE_PROPERTIES: return "VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES";
	case StructureType::e_EXPORT_FENCE_CREATE_INFO: return "VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO";
	case StructureType::e_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR: return "VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR";
	case StructureType::e_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR: return "VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR";
	case StructureType::e_FENCE_GET_WIN32_HANDLE_INFO_KHR: return "VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR";
	case StructureType::e_IMPORT_FENCE_FD_INFO_KHR: return "VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR";
	case StructureType::e_FENCE_GET_FD_INFO_KHR: return "VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR";
	case StructureType::e_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR";
	case StructureType::e_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR";
	case StructureType::e_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR: return "VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR";
	case StructureType::e_PERFORMANCE_QUERY_SUBMIT_INFO_KHR: return "VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR";
	case StructureType::e_ACQUIRE_PROFILING_LOCK_INFO_KHR: return "VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR";
	case StructureType::e_PERFORMANCE_COUNTER_KHR: return "VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR";
	case StructureType::e_PERFORMANCE_COUNTER_DESCRIPTION_KHR: return "VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR";
	case StructureType::e_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES";
	case StructureType::e_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO: return "VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO";
	case StructureType::e_IMAGE_VIEW_USAGE_CREATE_INFO: return "VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO";
	case StructureType::e_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO: return "VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO";
	case StructureType::e_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR";
	case StructureType::e_SURFACE_CAPABILITIES_2_KHR: return "VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR";
	case StructureType::e_SURFACE_FORMAT_2_KHR: return "VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR";
	case StructureType::e_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES";
	case StructureType::e_DISPLAY_PROPERTIES_2_KHR: return "VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR";
	case StructureType::e_DISPLAY_PLANE_PROPERTIES_2_KHR: return "VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR";
	case StructureType::e_DISPLAY_MODE_PROPERTIES_2_KHR: return "VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR";
	case StructureType::e_DISPLAY_PLANE_INFO_2_KHR: return "VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR";
	case StructureType::e_DISPLAY_PLANE_CAPABILITIES_2_KHR: return "VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR";
	case StructureType::e_IOS_SURFACE_CREATE_INFO_MVK: return "VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK";
	case StructureType::e_MACOS_SURFACE_CREATE_INFO_MVK: return "VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK";
	case StructureType::e_MEMORY_DEDICATED_REQUIREMENTS: return "VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS";
	case StructureType::e_MEMORY_DEDICATED_ALLOCATE_INFO: return "VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO";
	case StructureType::e_DEBUG_UTILS_OBJECT_NAME_INFO_EXT: return "VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT";
	case StructureType::e_DEBUG_UTILS_OBJECT_TAG_INFO_EXT: return "VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT";
	case StructureType::e_DEBUG_UTILS_LABEL_EXT: return "VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT";
	case StructureType::e_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT: return "VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT";
	case StructureType::e_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT";
	case StructureType::e_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID: return "VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID";
	case StructureType::e_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID: return "VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID";
	case StructureType::e_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID: return "VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID";
	case StructureType::e_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID: return "VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID";
	case StructureType::e_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID: return "VK_STRUCTURE_TYPE_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID";
	case StructureType::e_EXTERNAL_FORMAT_ANDROID: return "VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID";
	case StructureType::e_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES";
	case StructureType::e_SAMPLER_REDUCTION_MODE_CREATE_INFO: return "VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO";
	case StructureType::e_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES_EXT";
	case StructureType::e_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT: return "VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT";
	case StructureType::e_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO_EXT";
	case StructureType::e_SAMPLE_LOCATIONS_INFO_EXT: return "VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT";
	case StructureType::e_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT: return "VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT";
	case StructureType::e_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT";
	case StructureType::e_MULTISAMPLE_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT";
	case StructureType::e_PROTECTED_SUBMIT_INFO: return "VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO";
	case StructureType::e_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES";
	case StructureType::e_DEVICE_QUEUE_INFO_2: return "VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2";
	case StructureType::e_BUFFER_MEMORY_REQUIREMENTS_INFO_2: return "VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2";
	case StructureType::e_IMAGE_MEMORY_REQUIREMENTS_INFO_2: return "VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2";
	case StructureType::e_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2: return "VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2";
	case StructureType::e_MEMORY_REQUIREMENTS_2: return "VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2";
	case StructureType::e_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2: return "VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2";
	case StructureType::e_IMAGE_FORMAT_LIST_CREATE_INFO: return "VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO";
	case StructureType::e_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT";
	case StructureType::e_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT";
	case StructureType::e_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV";
	case StructureType::e_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV";
	case StructureType::e_SAMPLER_YCBCR_CONVERSION_CREATE_INFO: return "VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO";
	case StructureType::e_SAMPLER_YCBCR_CONVERSION_INFO: return "VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO";
	case StructureType::e_BIND_IMAGE_PLANE_MEMORY_INFO: return "VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO";
	case StructureType::e_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO: return "VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO";
	case StructureType::e_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES";
	case StructureType::e_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES: return "VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES";
	case StructureType::e_BIND_BUFFER_MEMORY_INFO: return "VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO";
	case StructureType::e_BIND_IMAGE_MEMORY_INFO: return "VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO";
	case StructureType::e_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT: return "VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT";
	case StructureType::e_DRM_FORMAT_MODIFIER_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT";
	case StructureType::e_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT";
	case StructureType::e_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT";
	case StructureType::e_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT";
	case StructureType::e_VALIDATION_CACHE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT";
	case StructureType::e_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT";
	case StructureType::e_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO: return "VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO";
	case StructureType::e_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES";
	case StructureType::e_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO: return "VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO";
	case StructureType::e_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT: return "VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT";
	case StructureType::e_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV";
	case StructureType::e_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV";
	case StructureType::e_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV";
	case StructureType::e_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV";
	case StructureType::e_RAY_TRACING_PIPELINE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV";
	case StructureType::e_ACCELERATION_STRUCTURE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV";
	case StructureType::e_GEOMETRY_NV: return "VK_STRUCTURE_TYPE_GEOMETRY_NV";
	case StructureType::e_GEOMETRY_TRIANGLES_NV: return "VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV";
	case StructureType::e_GEOMETRY_AABB_NV: return "VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV";
	case StructureType::e_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV: return "VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV";
	case StructureType::e_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV: return "VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV";
	case StructureType::e_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV: return "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV";
	case StructureType::e_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV";
	case StructureType::e_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV";
	case StructureType::e_ACCELERATION_STRUCTURE_INFO_NV: return "VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV";
	case StructureType::e_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV";
	case StructureType::e_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV";
	case StructureType::e_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES";
	case StructureType::e_DESCRIPTOR_SET_LAYOUT_SUPPORT: return "VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT";
	case StructureType::e_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT";
	case StructureType::e_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT";
	case StructureType::e_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES";
	case StructureType::e_IMPORT_MEMORY_HOST_POINTER_INFO_EXT: return "VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT";
	case StructureType::e_MEMORY_HOST_POINTER_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR";
	case StructureType::e_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD: return "VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD";
	case StructureType::e_CALIBRATED_TIMESTAMP_INFO_EXT: return "VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD";
	case StructureType::e_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD: return "VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD";
	case StructureType::e_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT";
	case StructureType::e_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT";
	case StructureType::e_PRESENT_FRAME_TOKEN_GGP: return "VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP";
	case StructureType::e_PIPELINE_CREATION_FEEDBACK_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_DRIVER_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES";
	case StructureType::e_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES";
	case StructureType::e_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES";
	case StructureType::e_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE: return "VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE";
	case StructureType::e_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV";
	case StructureType::e_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV";
	case StructureType::e_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV";
	case StructureType::e_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_NV";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV";
	case StructureType::e_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV";
	case StructureType::e_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV";
	case StructureType::e_CHECKPOINT_DATA_NV: return "VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV";
	case StructureType::e_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV: return "VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV";
	case StructureType::e_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES";
	case StructureType::e_SEMAPHORE_TYPE_CREATE_INFO: return "VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO";
	case StructureType::e_TIMELINE_SEMAPHORE_SUBMIT_INFO: return "VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO";
	case StructureType::e_SEMAPHORE_WAIT_INFO: return "VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO";
	case StructureType::e_SEMAPHORE_SIGNAL_INFO: return "VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL";
	case StructureType::e_QUERY_POOL_CREATE_INFO_INTEL: return "VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO_INTEL";
	case StructureType::e_INITIALIZE_PERFORMANCE_API_INFO_INTEL: return "VK_STRUCTURE_TYPE_INITIALIZE_PERFORMANCE_API_INFO_INTEL";
	case StructureType::e_PERFORMANCE_MARKER_INFO_INTEL: return "VK_STRUCTURE_TYPE_PERFORMANCE_MARKER_INFO_INTEL";
	case StructureType::e_PERFORMANCE_STREAM_MARKER_INFO_INTEL: return "VK_STRUCTURE_TYPE_PERFORMANCE_STREAM_MARKER_INFO_INTEL";
	case StructureType::e_PERFORMANCE_OVERRIDE_INFO_INTEL: return "VK_STRUCTURE_TYPE_PERFORMANCE_OVERRIDE_INFO_INTEL";
	case StructureType::e_PERFORMANCE_CONFIGURATION_ACQUIRE_INFO_INTEL: return "VK_STRUCTURE_TYPE_PERFORMANCE_CONFIGURATION_ACQUIRE_INFO_INTEL";
	case StructureType::e_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT";
	case StructureType::e_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD: return "VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD";
	case StructureType::e_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD: return "VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD";
	case StructureType::e_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA: return "VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA";
	case StructureType::e_METAL_SURFACE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT";
	case StructureType::e_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT";
	case StructureType::e_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD";
	case StructureType::e_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD";
	case StructureType::e_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT";
	case StructureType::e_MEMORY_PRIORITY_ALLOCATE_INFO_EXT: return "VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT";
	case StructureType::e_SURFACE_PROTECTED_CAPABILITIES_KHR: return "VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR";
	case StructureType::e_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV";
	case StructureType::e_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES";
	case StructureType::e_ATTACHMENT_REFERENCE_STENCIL_LAYOUT: return "VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT";
	case StructureType::e_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT: return "VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT";
	case StructureType::e_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT";
	case StructureType::e_BUFFER_DEVICE_ADDRESS_INFO: return "VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO";
	case StructureType::e_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT";
	case StructureType::e_IMAGE_STENCIL_USAGE_CREATE_INFO: return "VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO";
	case StructureType::e_VALIDATION_FEATURES_EXT: return "VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV";
	case StructureType::e_COOPERATIVE_MATRIX_PROPERTIES_NV: return "VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_NV";
	case StructureType::e_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV";
	case StructureType::e_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV";
	case StructureType::e_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV: return "VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV";
	case StructureType::e_FRAMEBUFFER_MIXED_SAMPLES_COMBINATION_NV: return "VK_STRUCTURE_TYPE_FRAMEBUFFER_MIXED_SAMPLES_COMBINATION_NV";
	case StructureType::e_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES";
	case StructureType::e_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT: return "VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT";
	case StructureType::e_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT: return "VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT";
	case StructureType::e_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT: return "VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT";
	case StructureType::e_HEADLESS_SURFACE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES";
	case StructureType::e_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO: return "VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO";
	case StructureType::e_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO: return "VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO";
	case StructureType::e_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO: return "VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO";
	case StructureType::e_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT";
	case StructureType::e_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT: return "VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT";
	case StructureType::e_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES";
	case StructureType::e_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR";
	case StructureType::e_PIPELINE_INFO_KHR: return "VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR";
	case StructureType::e_PIPELINE_EXECUTABLE_PROPERTIES_KHR: return "VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR";
	case StructureType::e_PIPELINE_EXECUTABLE_INFO_KHR: return "VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR";
	case StructureType::e_PIPELINE_EXECUTABLE_STATISTIC_KHR: return "VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR";
	case StructureType::e_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR: return "VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR";
	case StructureType::e_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT";
	case StructureType::e_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES_EXT: return "VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES_EXT";
	default: return "invalid";
	}
}

enum class SubpassContents
{
	e_INLINE = VK_SUBPASS_CONTENTS_INLINE,
	e_SECONDARY_COMMAND_BUFFERS = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
	e_BEGIN_RANGE = VK_SUBPASS_CONTENTS_BEGIN_RANGE,
	e_END_RANGE = VK_SUBPASS_CONTENTS_END_RANGE,
	e_RANGE_SIZE = VK_SUBPASS_CONTENTS_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SubpassContents)
inline std::string to_string(SubpassContents value)
{
	switch(value)
	{
	case SubpassContents::e_INLINE: return "VK_SUBPASS_CONTENTS_INLINE";
	case SubpassContents::e_SECONDARY_COMMAND_BUFFERS: return "VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS";
	default: return "invalid";
	}
}

enum class Result
{
	e_SUCCESS = VK_SUCCESS,
	e_NOT_READY = VK_NOT_READY,
	e_TIMEOUT = VK_TIMEOUT,
	e_EVENT_SET = VK_EVENT_SET,
	e_EVENT_RESET = VK_EVENT_RESET,
	e_INCOMPLETE = VK_INCOMPLETE,
	e_SUBOPTIMAL_KHR = VK_SUBOPTIMAL_KHR,
	e_ERROR_OUT_OF_HOST_MEMORY = VK_ERROR_OUT_OF_HOST_MEMORY,
	e_ERROR_OUT_OF_DEVICE_MEMORY = VK_ERROR_OUT_OF_DEVICE_MEMORY,
	e_ERROR_INITIALIZATION_FAILED = VK_ERROR_INITIALIZATION_FAILED,
	e_ERROR_DEVICE_LOST = VK_ERROR_DEVICE_LOST,
	e_ERROR_MEMORY_MAP_FAILED = VK_ERROR_MEMORY_MAP_FAILED,
	e_ERROR_LAYER_NOT_PRESENT = VK_ERROR_LAYER_NOT_PRESENT,
	e_ERROR_EXTENSION_NOT_PRESENT = VK_ERROR_EXTENSION_NOT_PRESENT,
	e_ERROR_FEATURE_NOT_PRESENT = VK_ERROR_FEATURE_NOT_PRESENT,
	e_ERROR_INCOMPATIBLE_DRIVER = VK_ERROR_INCOMPATIBLE_DRIVER,
	e_ERROR_TOO_MANY_OBJECTS = VK_ERROR_TOO_MANY_OBJECTS,
	e_ERROR_FORMAT_NOT_SUPPORTED = VK_ERROR_FORMAT_NOT_SUPPORTED,
	e_ERROR_FRAGMENTED_POOL = VK_ERROR_FRAGMENTED_POOL,
	e_ERROR_UNKNOWN = VK_ERROR_UNKNOWN,
	e_ERROR_SURFACE_LOST_KHR = VK_ERROR_SURFACE_LOST_KHR,
	e_ERROR_NATIVE_WINDOW_IN_USE_KHR = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
	e_ERROR_OUT_OF_DATE_KHR = VK_ERROR_OUT_OF_DATE_KHR,
	e_ERROR_INCOMPATIBLE_DISPLAY_KHR = VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
	e_ERROR_VALIDATION_FAILED_EXT = VK_ERROR_VALIDATION_FAILED_EXT,
	e_ERROR_INVALID_SHADER_NV = VK_ERROR_INVALID_SHADER_NV,
	e_ERROR_OUT_OF_POOL_MEMORY = VK_ERROR_OUT_OF_POOL_MEMORY,
	e_ERROR_INVALID_EXTERNAL_HANDLE = VK_ERROR_INVALID_EXTERNAL_HANDLE,
	e_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT = VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
	e_ERROR_FRAGMENTATION = VK_ERROR_FRAGMENTATION,
	e_ERROR_NOT_PERMITTED_EXT = VK_ERROR_NOT_PERMITTED_EXT,
	e_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT = VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
	e_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
	e_ERROR_OUT_OF_POOL_MEMORY_KHR = VK_ERROR_OUT_OF_POOL_MEMORY_KHR,
	e_ERROR_INVALID_EXTERNAL_HANDLE_KHR = VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR,
	e_ERROR_FRAGMENTATION_EXT = VK_ERROR_FRAGMENTATION_EXT,
	e_ERROR_INVALID_DEVICE_ADDRESS_EXT = VK_ERROR_INVALID_DEVICE_ADDRESS_EXT,
	e_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR,
	e_BEGIN_RANGE = VK_RESULT_BEGIN_RANGE,
	e_END_RANGE = VK_RESULT_END_RANGE,
	e_RANGE_SIZE = VK_RESULT_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(Result)
inline std::string to_string(Result value)
{
	switch(value)
	{
	case Result::e_SUCCESS: return "VK_SUCCESS";
	case Result::e_NOT_READY: return "VK_NOT_READY";
	case Result::e_TIMEOUT: return "VK_TIMEOUT";
	case Result::e_EVENT_SET: return "VK_EVENT_SET";
	case Result::e_EVENT_RESET: return "VK_EVENT_RESET";
	case Result::e_INCOMPLETE: return "VK_INCOMPLETE";
	case Result::e_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
	case Result::e_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case Result::e_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case Result::e_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
	case Result::e_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
	case Result::e_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
	case Result::e_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
	case Result::e_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case Result::e_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case Result::e_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case Result::e_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
	case Result::e_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case Result::e_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
	case Result::e_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
	case Result::e_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
	case Result::e_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case Result::e_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
	case Result::e_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case Result::e_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
	case Result::e_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
	case Result::e_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case Result::e_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case Result::e_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case Result::e_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
	case Result::e_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
	case Result::e_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case Result::e_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
	default: return "invalid";
	}
}

enum class DynamicState
{
	e_VIEWPORT = VK_DYNAMIC_STATE_VIEWPORT,
	e_SCISSOR = VK_DYNAMIC_STATE_SCISSOR,
	e_LINE_WIDTH = VK_DYNAMIC_STATE_LINE_WIDTH,
	e_DEPTH_BIAS = VK_DYNAMIC_STATE_DEPTH_BIAS,
	e_BLEND_CONSTANTS = VK_DYNAMIC_STATE_BLEND_CONSTANTS,
	e_DEPTH_BOUNDS = VK_DYNAMIC_STATE_DEPTH_BOUNDS,
	e_STENCIL_COMPARE_MASK = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
	e_STENCIL_WRITE_MASK = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
	e_STENCIL_REFERENCE = VK_DYNAMIC_STATE_STENCIL_REFERENCE,
	e_VIEWPORT_W_SCALING_NV = VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV,
	e_DISCARD_RECTANGLE_EXT = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT,
	e_SAMPLE_LOCATIONS_EXT = VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT,
	e_VIEWPORT_SHADING_RATE_PALETTE_NV = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV,
	e_VIEWPORT_COARSE_SAMPLE_ORDER_NV = VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV,
	e_EXCLUSIVE_SCISSOR_NV = VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV,
	e_LINE_STIPPLE_EXT = VK_DYNAMIC_STATE_LINE_STIPPLE_EXT,
	e_BEGIN_RANGE = VK_DYNAMIC_STATE_BEGIN_RANGE,
	e_END_RANGE = VK_DYNAMIC_STATE_END_RANGE,
	e_RANGE_SIZE = VK_DYNAMIC_STATE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DynamicState)
inline std::string to_string(DynamicState value)
{
	switch(value)
	{
	case DynamicState::e_VIEWPORT: return "VK_DYNAMIC_STATE_VIEWPORT";
	case DynamicState::e_SCISSOR: return "VK_DYNAMIC_STATE_SCISSOR";
	case DynamicState::e_LINE_WIDTH: return "VK_DYNAMIC_STATE_LINE_WIDTH";
	case DynamicState::e_DEPTH_BIAS: return "VK_DYNAMIC_STATE_DEPTH_BIAS";
	case DynamicState::e_BLEND_CONSTANTS: return "VK_DYNAMIC_STATE_BLEND_CONSTANTS";
	case DynamicState::e_DEPTH_BOUNDS: return "VK_DYNAMIC_STATE_DEPTH_BOUNDS";
	case DynamicState::e_STENCIL_COMPARE_MASK: return "VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK";
	case DynamicState::e_STENCIL_WRITE_MASK: return "VK_DYNAMIC_STATE_STENCIL_WRITE_MASK";
	case DynamicState::e_STENCIL_REFERENCE: return "VK_DYNAMIC_STATE_STENCIL_REFERENCE";
	case DynamicState::e_VIEWPORT_W_SCALING_NV: return "VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV";
	case DynamicState::e_DISCARD_RECTANGLE_EXT: return "VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT";
	case DynamicState::e_SAMPLE_LOCATIONS_EXT: return "VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT";
	case DynamicState::e_VIEWPORT_SHADING_RATE_PALETTE_NV: return "VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV";
	case DynamicState::e_VIEWPORT_COARSE_SAMPLE_ORDER_NV: return "VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV";
	case DynamicState::e_EXCLUSIVE_SCISSOR_NV: return "VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV";
	case DynamicState::e_LINE_STIPPLE_EXT: return "VK_DYNAMIC_STATE_LINE_STIPPLE_EXT";
	default: return "invalid";
	}
}

enum class DescriptorUpdateTemplateType
{
	e_DESCRIPTOR_SET = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
	e_PUSH_DESCRIPTORS_KHR = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR,
	e_DESCRIPTOR_SET_KHR = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET_KHR,
	e_BEGIN_RANGE = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DescriptorUpdateTemplateType)
inline std::string to_string(DescriptorUpdateTemplateType value)
{
	switch(value)
	{
	case DescriptorUpdateTemplateType::e_DESCRIPTOR_SET: return "VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET";
	case DescriptorUpdateTemplateType::e_PUSH_DESCRIPTORS_KHR: return "VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR";
	default: return "invalid";
	}
}

enum class ObjectType
{
	e_UNKNOWN = VK_OBJECT_TYPE_UNKNOWN,
	e_INSTANCE = VK_OBJECT_TYPE_INSTANCE,
	e_PHYSICAL_DEVICE = VK_OBJECT_TYPE_PHYSICAL_DEVICE,
	e_DEVICE = VK_OBJECT_TYPE_DEVICE,
	e_QUEUE = VK_OBJECT_TYPE_QUEUE,
	e_SEMAPHORE = VK_OBJECT_TYPE_SEMAPHORE,
	e_COMMAND_BUFFER = VK_OBJECT_TYPE_COMMAND_BUFFER,
	e_FENCE = VK_OBJECT_TYPE_FENCE,
	e_DEVICE_MEMORY = VK_OBJECT_TYPE_DEVICE_MEMORY,
	e_BUFFER = VK_OBJECT_TYPE_BUFFER,
	e_IMAGE = VK_OBJECT_TYPE_IMAGE,
	e_EVENT = VK_OBJECT_TYPE_EVENT,
	e_QUERY_POOL = VK_OBJECT_TYPE_QUERY_POOL,
	e_BUFFER_VIEW = VK_OBJECT_TYPE_BUFFER_VIEW,
	e_IMAGE_VIEW = VK_OBJECT_TYPE_IMAGE_VIEW,
	e_SHADER_MODULE = VK_OBJECT_TYPE_SHADER_MODULE,
	e_PIPELINE_CACHE = VK_OBJECT_TYPE_PIPELINE_CACHE,
	e_PIPELINE_LAYOUT = VK_OBJECT_TYPE_PIPELINE_LAYOUT,
	e_RENDER_PASS = VK_OBJECT_TYPE_RENDER_PASS,
	e_PIPELINE = VK_OBJECT_TYPE_PIPELINE,
	e_DESCRIPTOR_SET_LAYOUT = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
	e_SAMPLER = VK_OBJECT_TYPE_SAMPLER,
	e_DESCRIPTOR_POOL = VK_OBJECT_TYPE_DESCRIPTOR_POOL,
	e_DESCRIPTOR_SET = VK_OBJECT_TYPE_DESCRIPTOR_SET,
	e_FRAMEBUFFER = VK_OBJECT_TYPE_FRAMEBUFFER,
	e_COMMAND_POOL = VK_OBJECT_TYPE_COMMAND_POOL,
	e_SURFACE_KHR = VK_OBJECT_TYPE_SURFACE_KHR,
	e_SWAPCHAIN_KHR = VK_OBJECT_TYPE_SWAPCHAIN_KHR,
	e_DISPLAY_KHR = VK_OBJECT_TYPE_DISPLAY_KHR,
	e_DISPLAY_MODE_KHR = VK_OBJECT_TYPE_DISPLAY_MODE_KHR,
	e_DEBUG_REPORT_CALLBACK_EXT = VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT,
	e_DESCRIPTOR_UPDATE_TEMPLATE = VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE,
	e_OBJECT_TABLE_NVX = VK_OBJECT_TYPE_OBJECT_TABLE_NVX,
	e_INDIRECT_COMMANDS_LAYOUT_NVX = VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX,
	e_DEBUG_UTILS_MESSENGER_EXT = VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT,
	e_SAMPLER_YCBCR_CONVERSION = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION,
	e_VALIDATION_CACHE_EXT = VK_OBJECT_TYPE_VALIDATION_CACHE_EXT,
	e_ACCELERATION_STRUCTURE_NV = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV,
	e_PERFORMANCE_CONFIGURATION_INTEL = VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL,
	e_DESCRIPTOR_UPDATE_TEMPLATE_KHR = VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR,
	e_SAMPLER_YCBCR_CONVERSION_KHR = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR,
	e_BEGIN_RANGE = VK_OBJECT_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_OBJECT_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_OBJECT_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ObjectType)
inline std::string to_string(ObjectType value)
{
	switch(value)
	{
	case ObjectType::e_UNKNOWN: return "VK_OBJECT_TYPE_UNKNOWN";
	case ObjectType::e_INSTANCE: return "VK_OBJECT_TYPE_INSTANCE";
	case ObjectType::e_PHYSICAL_DEVICE: return "VK_OBJECT_TYPE_PHYSICAL_DEVICE";
	case ObjectType::e_DEVICE: return "VK_OBJECT_TYPE_DEVICE";
	case ObjectType::e_QUEUE: return "VK_OBJECT_TYPE_QUEUE";
	case ObjectType::e_SEMAPHORE: return "VK_OBJECT_TYPE_SEMAPHORE";
	case ObjectType::e_COMMAND_BUFFER: return "VK_OBJECT_TYPE_COMMAND_BUFFER";
	case ObjectType::e_FENCE: return "VK_OBJECT_TYPE_FENCE";
	case ObjectType::e_DEVICE_MEMORY: return "VK_OBJECT_TYPE_DEVICE_MEMORY";
	case ObjectType::e_BUFFER: return "VK_OBJECT_TYPE_BUFFER";
	case ObjectType::e_IMAGE: return "VK_OBJECT_TYPE_IMAGE";
	case ObjectType::e_EVENT: return "VK_OBJECT_TYPE_EVENT";
	case ObjectType::e_QUERY_POOL: return "VK_OBJECT_TYPE_QUERY_POOL";
	case ObjectType::e_BUFFER_VIEW: return "VK_OBJECT_TYPE_BUFFER_VIEW";
	case ObjectType::e_IMAGE_VIEW: return "VK_OBJECT_TYPE_IMAGE_VIEW";
	case ObjectType::e_SHADER_MODULE: return "VK_OBJECT_TYPE_SHADER_MODULE";
	case ObjectType::e_PIPELINE_CACHE: return "VK_OBJECT_TYPE_PIPELINE_CACHE";
	case ObjectType::e_PIPELINE_LAYOUT: return "VK_OBJECT_TYPE_PIPELINE_LAYOUT";
	case ObjectType::e_RENDER_PASS: return "VK_OBJECT_TYPE_RENDER_PASS";
	case ObjectType::e_PIPELINE: return "VK_OBJECT_TYPE_PIPELINE";
	case ObjectType::e_DESCRIPTOR_SET_LAYOUT: return "VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT";
	case ObjectType::e_SAMPLER: return "VK_OBJECT_TYPE_SAMPLER";
	case ObjectType::e_DESCRIPTOR_POOL: return "VK_OBJECT_TYPE_DESCRIPTOR_POOL";
	case ObjectType::e_DESCRIPTOR_SET: return "VK_OBJECT_TYPE_DESCRIPTOR_SET";
	case ObjectType::e_FRAMEBUFFER: return "VK_OBJECT_TYPE_FRAMEBUFFER";
	case ObjectType::e_COMMAND_POOL: return "VK_OBJECT_TYPE_COMMAND_POOL";
	case ObjectType::e_SURFACE_KHR: return "VK_OBJECT_TYPE_SURFACE_KHR";
	case ObjectType::e_SWAPCHAIN_KHR: return "VK_OBJECT_TYPE_SWAPCHAIN_KHR";
	case ObjectType::e_DISPLAY_KHR: return "VK_OBJECT_TYPE_DISPLAY_KHR";
	case ObjectType::e_DISPLAY_MODE_KHR: return "VK_OBJECT_TYPE_DISPLAY_MODE_KHR";
	case ObjectType::e_DEBUG_REPORT_CALLBACK_EXT: return "VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT";
	case ObjectType::e_DESCRIPTOR_UPDATE_TEMPLATE: return "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE";
	case ObjectType::e_OBJECT_TABLE_NVX: return "VK_OBJECT_TYPE_OBJECT_TABLE_NVX";
	case ObjectType::e_INDIRECT_COMMANDS_LAYOUT_NVX: return "VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX";
	case ObjectType::e_DEBUG_UTILS_MESSENGER_EXT: return "VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT";
	case ObjectType::e_SAMPLER_YCBCR_CONVERSION: return "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION";
	case ObjectType::e_VALIDATION_CACHE_EXT: return "VK_OBJECT_TYPE_VALIDATION_CACHE_EXT";
	case ObjectType::e_ACCELERATION_STRUCTURE_NV: return "VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV";
	case ObjectType::e_PERFORMANCE_CONFIGURATION_INTEL: return "VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL";
	default: return "invalid";
	}
}

enum class SemaphoreType
{
	e_BINARY = VK_SEMAPHORE_TYPE_BINARY,
	e_TIMELINE = VK_SEMAPHORE_TYPE_TIMELINE,
	e_BINARY_KHR = VK_SEMAPHORE_TYPE_BINARY_KHR,
	e_TIMELINE_KHR = VK_SEMAPHORE_TYPE_TIMELINE_KHR,
	e_BEGIN_RANGE = VK_SEMAPHORE_TYPE_BEGIN_RANGE,
	e_END_RANGE = VK_SEMAPHORE_TYPE_END_RANGE,
	e_RANGE_SIZE = VK_SEMAPHORE_TYPE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SemaphoreType)
inline std::string to_string(SemaphoreType value)
{
	switch(value)
	{
	case SemaphoreType::e_BINARY: return "VK_SEMAPHORE_TYPE_BINARY";
	case SemaphoreType::e_TIMELINE: return "VK_SEMAPHORE_TYPE_TIMELINE";
	default: return "invalid";
	}
}

enum class PresentModeKHR
{
	e_IMMEDIATE_KHR = VK_PRESENT_MODE_IMMEDIATE_KHR,
	e_MAILBOX_KHR = VK_PRESENT_MODE_MAILBOX_KHR,
	e_FIFO_KHR = VK_PRESENT_MODE_FIFO_KHR,
	e_FIFO_RELAXED_KHR = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
	e_SHARED_DEMAND_REFRESH_KHR = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
	e_SHARED_CONTINUOUS_REFRESH_KHR = VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
	e_BEGIN_RANGE = VK_PRESENT_MODE_BEGIN_RANGE_KHR,
	e_END_RANGE = VK_PRESENT_MODE_END_RANGE_KHR,
	e_RANGE_SIZE = VK_PRESENT_MODE_RANGE_SIZE_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PresentModeKHR)
inline std::string to_string(PresentModeKHR value)
{
	switch(value)
	{
	case PresentModeKHR::e_IMMEDIATE_KHR: return "VK_PRESENT_MODE_IMMEDIATE_KHR";
	case PresentModeKHR::e_MAILBOX_KHR: return "VK_PRESENT_MODE_MAILBOX_KHR";
	case PresentModeKHR::e_FIFO_KHR: return "VK_PRESENT_MODE_FIFO_KHR";
	case PresentModeKHR::e_FIFO_RELAXED_KHR: return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
	case PresentModeKHR::e_SHARED_DEMAND_REFRESH_KHR: return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
	case PresentModeKHR::e_SHARED_CONTINUOUS_REFRESH_KHR: return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
	default: return "invalid";
	}
}

enum class ColorSpaceKHR
{
	e_SRGB_NONLINEAR_KHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	e_DISPLAY_P3_NONLINEAR_EXT = VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT,
	e_EXTENDED_SRGB_LINEAR_EXT = VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT,
	e_DISPLAY_P3_LINEAR_EXT = VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,
	e_DCI_P3_NONLINEAR_EXT = VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,
	e_BT709_LINEAR_EXT = VK_COLOR_SPACE_BT709_LINEAR_EXT,
	e_BT709_NONLINEAR_EXT = VK_COLOR_SPACE_BT709_NONLINEAR_EXT,
	e_BT2020_LINEAR_EXT = VK_COLOR_SPACE_BT2020_LINEAR_EXT,
	e_HDR10_ST2084_EXT = VK_COLOR_SPACE_HDR10_ST2084_EXT,
	e_DOLBYVISION_EXT = VK_COLOR_SPACE_DOLBYVISION_EXT,
	e_HDR10_HLG_EXT = VK_COLOR_SPACE_HDR10_HLG_EXT,
	e_ADOBERGB_LINEAR_EXT = VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,
	e_ADOBERGB_NONLINEAR_EXT = VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT,
	e_PASS_THROUGH_EXT = VK_COLOR_SPACE_PASS_THROUGH_EXT,
	e_EXTENDED_SRGB_NONLINEAR_EXT = VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT,
	e_DISPLAY_NATIVE_AMD = VK_COLOR_SPACE_DISPLAY_NATIVE_AMD,
	e_DCI_P3_LINEAR_EXT = VK_COLOR_SPACE_DCI_P3_LINEAR_EXT,
	e_BEGIN_RANGE = VK_COLOR_SPACE_BEGIN_RANGE_KHR,
	e_END_RANGE = VK_COLOR_SPACE_END_RANGE_KHR,
	e_RANGE_SIZE = VK_COLOR_SPACE_RANGE_SIZE_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ColorSpaceKHR)
inline std::string to_string(ColorSpaceKHR value)
{
	switch(value)
	{
	case ColorSpaceKHR::e_SRGB_NONLINEAR_KHR: return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR";
	case ColorSpaceKHR::e_DISPLAY_P3_NONLINEAR_EXT: return "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT";
	case ColorSpaceKHR::e_EXTENDED_SRGB_LINEAR_EXT: return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT";
	case ColorSpaceKHR::e_DISPLAY_P3_LINEAR_EXT: return "VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT";
	case ColorSpaceKHR::e_DCI_P3_NONLINEAR_EXT: return "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT";
	case ColorSpaceKHR::e_BT709_LINEAR_EXT: return "VK_COLOR_SPACE_BT709_LINEAR_EXT";
	case ColorSpaceKHR::e_BT709_NONLINEAR_EXT: return "VK_COLOR_SPACE_BT709_NONLINEAR_EXT";
	case ColorSpaceKHR::e_BT2020_LINEAR_EXT: return "VK_COLOR_SPACE_BT2020_LINEAR_EXT";
	case ColorSpaceKHR::e_HDR10_ST2084_EXT: return "VK_COLOR_SPACE_HDR10_ST2084_EXT";
	case ColorSpaceKHR::e_DOLBYVISION_EXT: return "VK_COLOR_SPACE_DOLBYVISION_EXT";
	case ColorSpaceKHR::e_HDR10_HLG_EXT: return "VK_COLOR_SPACE_HDR10_HLG_EXT";
	case ColorSpaceKHR::e_ADOBERGB_LINEAR_EXT: return "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT";
	case ColorSpaceKHR::e_ADOBERGB_NONLINEAR_EXT: return "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT";
	case ColorSpaceKHR::e_PASS_THROUGH_EXT: return "VK_COLOR_SPACE_PASS_THROUGH_EXT";
	case ColorSpaceKHR::e_EXTENDED_SRGB_NONLINEAR_EXT: return "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT";
	case ColorSpaceKHR::e_DISPLAY_NATIVE_AMD: return "VK_COLOR_SPACE_DISPLAY_NATIVE_AMD";
	default: return "invalid";
	}
}

enum class TimeDomainEXT
{
	e_DEVICE_EXT = VK_TIME_DOMAIN_DEVICE_EXT,
	e_CLOCK_MONOTONIC_EXT = VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT,
	e_CLOCK_MONOTONIC_RAW_EXT = VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT,
	e_QUERY_PERFORMANCE_COUNTER_EXT = VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT,
	e_BEGIN_RANGE = VK_TIME_DOMAIN_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_TIME_DOMAIN_END_RANGE_EXT,
	e_RANGE_SIZE = VK_TIME_DOMAIN_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(TimeDomainEXT)
inline std::string to_string(TimeDomainEXT value)
{
	switch(value)
	{
	case TimeDomainEXT::e_DEVICE_EXT: return "VK_TIME_DOMAIN_DEVICE_EXT";
	case TimeDomainEXT::e_CLOCK_MONOTONIC_EXT: return "VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT";
	case TimeDomainEXT::e_CLOCK_MONOTONIC_RAW_EXT: return "VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT";
	case TimeDomainEXT::e_QUERY_PERFORMANCE_COUNTER_EXT: return "VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT";
	default: return "invalid";
	}
}

enum class DebugReportObjectTypeEXT
{
	e_UNKNOWN_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,
	e_INSTANCE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,
	e_PHYSICAL_DEVICE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
	e_DEVICE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
	e_QUEUE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
	e_SEMAPHORE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,
	e_COMMAND_BUFFER_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
	e_FENCE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT,
	e_DEVICE_MEMORY_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
	e_BUFFER_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
	e_IMAGE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
	e_EVENT_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT,
	e_QUERY_POOL_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT,
	e_BUFFER_VIEW_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,
	e_IMAGE_VIEW_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
	e_SHADER_MODULE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
	e_PIPELINE_CACHE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT,
	e_PIPELINE_LAYOUT_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
	e_RENDER_PASS_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
	e_PIPELINE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
	e_DESCRIPTOR_SET_LAYOUT_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
	e_SAMPLER_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT,
	e_DESCRIPTOR_POOL_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
	e_DESCRIPTOR_SET_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
	e_FRAMEBUFFER_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT,
	e_COMMAND_POOL_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
	e_SURFACE_KHR_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT,
	e_SWAPCHAIN_KHR_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
	e_DEBUG_REPORT_CALLBACK_EXT_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT,
	e_DISPLAY_KHR_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT,
	e_DISPLAY_MODE_KHR_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT,
	e_OBJECT_TABLE_NVX_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT,
	e_INDIRECT_COMMANDS_LAYOUT_NVX_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT,
	e_VALIDATION_CACHE_EXT_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT,
	e_DESCRIPTOR_UPDATE_TEMPLATE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT,
	e_SAMPLER_YCBCR_CONVERSION_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT,
	e_ACCELERATION_STRUCTURE_NV_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
	e_DEBUG_REPORT_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT,
	e_VALIDATION_CACHE_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT,
	e_DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT,
	e_SAMPLER_YCBCR_CONVERSION_KHR_EXT = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR_EXT,
	e_BEGIN_RANGE = VK_DEBUG_REPORT_OBJECT_TYPE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_DEBUG_REPORT_OBJECT_TYPE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_DEBUG_REPORT_OBJECT_TYPE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DebugReportObjectTypeEXT)
inline std::string to_string(DebugReportObjectTypeEXT value)
{
	switch(value)
	{
	case DebugReportObjectTypeEXT::e_UNKNOWN_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT";
	case DebugReportObjectTypeEXT::e_INSTANCE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT";
	case DebugReportObjectTypeEXT::e_PHYSICAL_DEVICE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT";
	case DebugReportObjectTypeEXT::e_DEVICE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT";
	case DebugReportObjectTypeEXT::e_QUEUE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT";
	case DebugReportObjectTypeEXT::e_SEMAPHORE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT";
	case DebugReportObjectTypeEXT::e_COMMAND_BUFFER_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT";
	case DebugReportObjectTypeEXT::e_FENCE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT";
	case DebugReportObjectTypeEXT::e_DEVICE_MEMORY_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT";
	case DebugReportObjectTypeEXT::e_BUFFER_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT";
	case DebugReportObjectTypeEXT::e_IMAGE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT";
	case DebugReportObjectTypeEXT::e_EVENT_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT";
	case DebugReportObjectTypeEXT::e_QUERY_POOL_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT";
	case DebugReportObjectTypeEXT::e_BUFFER_VIEW_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT";
	case DebugReportObjectTypeEXT::e_IMAGE_VIEW_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT";
	case DebugReportObjectTypeEXT::e_SHADER_MODULE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT";
	case DebugReportObjectTypeEXT::e_PIPELINE_CACHE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT";
	case DebugReportObjectTypeEXT::e_PIPELINE_LAYOUT_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT";
	case DebugReportObjectTypeEXT::e_RENDER_PASS_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT";
	case DebugReportObjectTypeEXT::e_PIPELINE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT";
	case DebugReportObjectTypeEXT::e_DESCRIPTOR_SET_LAYOUT_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT";
	case DebugReportObjectTypeEXT::e_SAMPLER_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT";
	case DebugReportObjectTypeEXT::e_DESCRIPTOR_POOL_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT";
	case DebugReportObjectTypeEXT::e_DESCRIPTOR_SET_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT";
	case DebugReportObjectTypeEXT::e_FRAMEBUFFER_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT";
	case DebugReportObjectTypeEXT::e_COMMAND_POOL_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT";
	case DebugReportObjectTypeEXT::e_SURFACE_KHR_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT";
	case DebugReportObjectTypeEXT::e_SWAPCHAIN_KHR_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT";
	case DebugReportObjectTypeEXT::e_DEBUG_REPORT_CALLBACK_EXT_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT";
	case DebugReportObjectTypeEXT::e_DISPLAY_KHR_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT";
	case DebugReportObjectTypeEXT::e_DISPLAY_MODE_KHR_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT";
	case DebugReportObjectTypeEXT::e_OBJECT_TABLE_NVX_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT";
	case DebugReportObjectTypeEXT::e_INDIRECT_COMMANDS_LAYOUT_NVX_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT";
	case DebugReportObjectTypeEXT::e_VALIDATION_CACHE_EXT_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT";
	case DebugReportObjectTypeEXT::e_DESCRIPTOR_UPDATE_TEMPLATE_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT";
	case DebugReportObjectTypeEXT::e_SAMPLER_YCBCR_CONVERSION_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT";
	case DebugReportObjectTypeEXT::e_ACCELERATION_STRUCTURE_NV_EXT: return "VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT";
	default: return "invalid";
	}
}

enum class RasterizationOrderAMD
{
	e_STRICT_AMD = VK_RASTERIZATION_ORDER_STRICT_AMD,
	e_RELAXED_AMD = VK_RASTERIZATION_ORDER_RELAXED_AMD,
	e_BEGIN_RANGE = VK_RASTERIZATION_ORDER_BEGIN_RANGE_AMD,
	e_END_RANGE = VK_RASTERIZATION_ORDER_END_RANGE_AMD,
	e_RANGE_SIZE = VK_RASTERIZATION_ORDER_RANGE_SIZE_AMD,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(RasterizationOrderAMD)
inline std::string to_string(RasterizationOrderAMD value)
{
	switch(value)
	{
	case RasterizationOrderAMD::e_STRICT_AMD: return "VK_RASTERIZATION_ORDER_STRICT_AMD";
	case RasterizationOrderAMD::e_RELAXED_AMD: return "VK_RASTERIZATION_ORDER_RELAXED_AMD";
	default: return "invalid";
	}
}

enum class ValidationCheckEXT
{
	e_ALL_EXT = VK_VALIDATION_CHECK_ALL_EXT,
	e_SHADERS_EXT = VK_VALIDATION_CHECK_SHADERS_EXT,
	e_BEGIN_RANGE = VK_VALIDATION_CHECK_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_VALIDATION_CHECK_END_RANGE_EXT,
	e_RANGE_SIZE = VK_VALIDATION_CHECK_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ValidationCheckEXT)
inline std::string to_string(ValidationCheckEXT value)
{
	switch(value)
	{
	case ValidationCheckEXT::e_ALL_EXT: return "VK_VALIDATION_CHECK_ALL_EXT";
	case ValidationCheckEXT::e_SHADERS_EXT: return "VK_VALIDATION_CHECK_SHADERS_EXT";
	default: return "invalid";
	}
}

enum class ValidationFeatureEnableEXT
{
	e_GPU_ASSISTED_EXT = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
	e_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
	e_BEST_PRACTICES_EXT = VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
	e_BEGIN_RANGE = VK_VALIDATION_FEATURE_ENABLE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_VALIDATION_FEATURE_ENABLE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_VALIDATION_FEATURE_ENABLE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ValidationFeatureEnableEXT)
inline std::string to_string(ValidationFeatureEnableEXT value)
{
	switch(value)
	{
	case ValidationFeatureEnableEXT::e_GPU_ASSISTED_EXT: return "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT";
	case ValidationFeatureEnableEXT::e_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT: return "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT";
	case ValidationFeatureEnableEXT::e_BEST_PRACTICES_EXT: return "VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT";
	default: return "invalid";
	}
}

enum class ValidationFeatureDisableEXT
{
	e_ALL_EXT = VK_VALIDATION_FEATURE_DISABLE_ALL_EXT,
	e_SHADERS_EXT = VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT,
	e_THREAD_SAFETY_EXT = VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
	e_API_PARAMETERS_EXT = VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
	e_OBJECT_LIFETIMES_EXT = VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT,
	e_CORE_CHECKS_EXT = VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT,
	e_UNIQUE_HANDLES_EXT = VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT,
	e_BEGIN_RANGE = VK_VALIDATION_FEATURE_DISABLE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_VALIDATION_FEATURE_DISABLE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_VALIDATION_FEATURE_DISABLE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ValidationFeatureDisableEXT)
inline std::string to_string(ValidationFeatureDisableEXT value)
{
	switch(value)
	{
	case ValidationFeatureDisableEXT::e_ALL_EXT: return "VK_VALIDATION_FEATURE_DISABLE_ALL_EXT";
	case ValidationFeatureDisableEXT::e_SHADERS_EXT: return "VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT";
	case ValidationFeatureDisableEXT::e_THREAD_SAFETY_EXT: return "VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT";
	case ValidationFeatureDisableEXT::e_API_PARAMETERS_EXT: return "VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT";
	case ValidationFeatureDisableEXT::e_OBJECT_LIFETIMES_EXT: return "VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT";
	case ValidationFeatureDisableEXT::e_CORE_CHECKS_EXT: return "VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT";
	case ValidationFeatureDisableEXT::e_UNIQUE_HANDLES_EXT: return "VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT";
	default: return "invalid";
	}
}

enum class IndirectCommandsTokenTypeNVX
{
	e_PIPELINE_NVX = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PIPELINE_NVX,
	e_DESCRIPTOR_SET_NVX = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DESCRIPTOR_SET_NVX,
	e_INDEX_BUFFER_NVX = VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NVX,
	e_VERTEX_BUFFER_NVX = VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NVX,
	e_PUSH_CONSTANT_NVX = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NVX,
	e_DRAW_INDEXED_NVX = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NVX,
	e_DRAW_NVX = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NVX,
	e_DISPATCH_NVX = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_NVX,
	e_BEGIN_RANGE = VK_INDIRECT_COMMANDS_TOKEN_TYPE_BEGIN_RANGE_NVX,
	e_END_RANGE = VK_INDIRECT_COMMANDS_TOKEN_TYPE_END_RANGE_NVX,
	e_RANGE_SIZE = VK_INDIRECT_COMMANDS_TOKEN_TYPE_RANGE_SIZE_NVX,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(IndirectCommandsTokenTypeNVX)
inline std::string to_string(IndirectCommandsTokenTypeNVX value)
{
	switch(value)
	{
	case IndirectCommandsTokenTypeNVX::e_PIPELINE_NVX: return "VK_INDIRECT_COMMANDS_TOKEN_TYPE_PIPELINE_NVX";
	case IndirectCommandsTokenTypeNVX::e_DESCRIPTOR_SET_NVX: return "VK_INDIRECT_COMMANDS_TOKEN_TYPE_DESCRIPTOR_SET_NVX";
	case IndirectCommandsTokenTypeNVX::e_INDEX_BUFFER_NVX: return "VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NVX";
	case IndirectCommandsTokenTypeNVX::e_VERTEX_BUFFER_NVX: return "VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NVX";
	case IndirectCommandsTokenTypeNVX::e_PUSH_CONSTANT_NVX: return "VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NVX";
	case IndirectCommandsTokenTypeNVX::e_DRAW_INDEXED_NVX: return "VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NVX";
	case IndirectCommandsTokenTypeNVX::e_DRAW_NVX: return "VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NVX";
	case IndirectCommandsTokenTypeNVX::e_DISPATCH_NVX: return "VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_NVX";
	default: return "invalid";
	}
}

enum class ObjectEntryTypeNVX
{
	e_DESCRIPTOR_SET_NVX = VK_OBJECT_ENTRY_TYPE_DESCRIPTOR_SET_NVX,
	e_PIPELINE_NVX = VK_OBJECT_ENTRY_TYPE_PIPELINE_NVX,
	e_INDEX_BUFFER_NVX = VK_OBJECT_ENTRY_TYPE_INDEX_BUFFER_NVX,
	e_VERTEX_BUFFER_NVX = VK_OBJECT_ENTRY_TYPE_VERTEX_BUFFER_NVX,
	e_PUSH_CONSTANT_NVX = VK_OBJECT_ENTRY_TYPE_PUSH_CONSTANT_NVX,
	e_BEGIN_RANGE = VK_OBJECT_ENTRY_TYPE_BEGIN_RANGE_NVX,
	e_END_RANGE = VK_OBJECT_ENTRY_TYPE_END_RANGE_NVX,
	e_RANGE_SIZE = VK_OBJECT_ENTRY_TYPE_RANGE_SIZE_NVX,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ObjectEntryTypeNVX)
inline std::string to_string(ObjectEntryTypeNVX value)
{
	switch(value)
	{
	case ObjectEntryTypeNVX::e_DESCRIPTOR_SET_NVX: return "VK_OBJECT_ENTRY_TYPE_DESCRIPTOR_SET_NVX";
	case ObjectEntryTypeNVX::e_PIPELINE_NVX: return "VK_OBJECT_ENTRY_TYPE_PIPELINE_NVX";
	case ObjectEntryTypeNVX::e_INDEX_BUFFER_NVX: return "VK_OBJECT_ENTRY_TYPE_INDEX_BUFFER_NVX";
	case ObjectEntryTypeNVX::e_VERTEX_BUFFER_NVX: return "VK_OBJECT_ENTRY_TYPE_VERTEX_BUFFER_NVX";
	case ObjectEntryTypeNVX::e_PUSH_CONSTANT_NVX: return "VK_OBJECT_ENTRY_TYPE_PUSH_CONSTANT_NVX";
	default: return "invalid";
	}
}

enum class DisplayPowerStateEXT
{
	e_OFF_EXT = VK_DISPLAY_POWER_STATE_OFF_EXT,
	e_SUSPEND_EXT = VK_DISPLAY_POWER_STATE_SUSPEND_EXT,
	e_ON_EXT = VK_DISPLAY_POWER_STATE_ON_EXT,
	e_BEGIN_RANGE = VK_DISPLAY_POWER_STATE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_DISPLAY_POWER_STATE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_DISPLAY_POWER_STATE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DisplayPowerStateEXT)
inline std::string to_string(DisplayPowerStateEXT value)
{
	switch(value)
	{
	case DisplayPowerStateEXT::e_OFF_EXT: return "VK_DISPLAY_POWER_STATE_OFF_EXT";
	case DisplayPowerStateEXT::e_SUSPEND_EXT: return "VK_DISPLAY_POWER_STATE_SUSPEND_EXT";
	case DisplayPowerStateEXT::e_ON_EXT: return "VK_DISPLAY_POWER_STATE_ON_EXT";
	default: return "invalid";
	}
}

enum class DeviceEventTypeEXT
{
	e_DISPLAY_HOTPLUG_EXT = VK_DEVICE_EVENT_TYPE_DISPLAY_HOTPLUG_EXT,
	e_BEGIN_RANGE = VK_DEVICE_EVENT_TYPE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_DEVICE_EVENT_TYPE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_DEVICE_EVENT_TYPE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DeviceEventTypeEXT)
inline std::string to_string(DeviceEventTypeEXT value)
{
	switch(value)
	{
	case DeviceEventTypeEXT::e_DISPLAY_HOTPLUG_EXT: return "VK_DEVICE_EVENT_TYPE_DISPLAY_HOTPLUG_EXT";
	default: return "invalid";
	}
}

enum class DisplayEventTypeEXT
{
	e_FIRST_PIXEL_OUT_EXT = VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT,
	e_BEGIN_RANGE = VK_DISPLAY_EVENT_TYPE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_DISPLAY_EVENT_TYPE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_DISPLAY_EVENT_TYPE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DisplayEventTypeEXT)
inline std::string to_string(DisplayEventTypeEXT value)
{
	switch(value)
	{
	case DisplayEventTypeEXT::e_FIRST_PIXEL_OUT_EXT: return "VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT";
	default: return "invalid";
	}
}

enum class ViewportCoordinateSwizzleNV
{
	e_POSITIVE_X_NV = VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV,
	e_NEGATIVE_X_NV = VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_X_NV,
	e_POSITIVE_Y_NV = VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV,
	e_NEGATIVE_Y_NV = VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Y_NV,
	e_POSITIVE_Z_NV = VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV,
	e_NEGATIVE_Z_NV = VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Z_NV,
	e_POSITIVE_W_NV = VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV,
	e_NEGATIVE_W_NV = VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_W_NV,
	e_BEGIN_RANGE = VK_VIEWPORT_COORDINATE_SWIZZLE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_VIEWPORT_COORDINATE_SWIZZLE_END_RANGE_NV,
	e_RANGE_SIZE = VK_VIEWPORT_COORDINATE_SWIZZLE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ViewportCoordinateSwizzleNV)
inline std::string to_string(ViewportCoordinateSwizzleNV value)
{
	switch(value)
	{
	case ViewportCoordinateSwizzleNV::e_POSITIVE_X_NV: return "VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV";
	case ViewportCoordinateSwizzleNV::e_NEGATIVE_X_NV: return "VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_X_NV";
	case ViewportCoordinateSwizzleNV::e_POSITIVE_Y_NV: return "VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV";
	case ViewportCoordinateSwizzleNV::e_NEGATIVE_Y_NV: return "VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Y_NV";
	case ViewportCoordinateSwizzleNV::e_POSITIVE_Z_NV: return "VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV";
	case ViewportCoordinateSwizzleNV::e_NEGATIVE_Z_NV: return "VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Z_NV";
	case ViewportCoordinateSwizzleNV::e_POSITIVE_W_NV: return "VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV";
	case ViewportCoordinateSwizzleNV::e_NEGATIVE_W_NV: return "VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_W_NV";
	default: return "invalid";
	}
}

enum class DiscardRectangleModeEXT
{
	e_INCLUSIVE_EXT = VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT,
	e_EXCLUSIVE_EXT = VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT,
	e_BEGIN_RANGE = VK_DISCARD_RECTANGLE_MODE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_DISCARD_RECTANGLE_MODE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_DISCARD_RECTANGLE_MODE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DiscardRectangleModeEXT)
inline std::string to_string(DiscardRectangleModeEXT value)
{
	switch(value)
	{
	case DiscardRectangleModeEXT::e_INCLUSIVE_EXT: return "VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT";
	case DiscardRectangleModeEXT::e_EXCLUSIVE_EXT: return "VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT";
	default: return "invalid";
	}
}

enum class PointClippingBehavior
{
	e_ALL_CLIP_PLANES = VK_POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES,
	e_USER_CLIP_PLANES_ONLY = VK_POINT_CLIPPING_BEHAVIOR_USER_CLIP_PLANES_ONLY,
	e_ALL_CLIP_PLANES_KHR = VK_POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES_KHR,
	e_USER_CLIP_PLANES_ONLY_KHR = VK_POINT_CLIPPING_BEHAVIOR_USER_CLIP_PLANES_ONLY_KHR,
	e_BEGIN_RANGE = VK_POINT_CLIPPING_BEHAVIOR_BEGIN_RANGE,
	e_END_RANGE = VK_POINT_CLIPPING_BEHAVIOR_END_RANGE,
	e_RANGE_SIZE = VK_POINT_CLIPPING_BEHAVIOR_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PointClippingBehavior)
inline std::string to_string(PointClippingBehavior value)
{
	switch(value)
	{
	case PointClippingBehavior::e_ALL_CLIP_PLANES: return "VK_POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES";
	case PointClippingBehavior::e_USER_CLIP_PLANES_ONLY: return "VK_POINT_CLIPPING_BEHAVIOR_USER_CLIP_PLANES_ONLY";
	default: return "invalid";
	}
}

enum class SamplerReductionMode
{
	e_WEIGHTED_AVERAGE = VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE,
	e_MIN = VK_SAMPLER_REDUCTION_MODE_MIN,
	e_MAX = VK_SAMPLER_REDUCTION_MODE_MAX,
	e_WEIGHTED_AVERAGE_EXT = VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_EXT,
	e_MIN_EXT = VK_SAMPLER_REDUCTION_MODE_MIN_EXT,
	e_MAX_EXT = VK_SAMPLER_REDUCTION_MODE_MAX_EXT,
	e_BEGIN_RANGE = VK_SAMPLER_REDUCTION_MODE_BEGIN_RANGE,
	e_END_RANGE = VK_SAMPLER_REDUCTION_MODE_END_RANGE,
	e_RANGE_SIZE = VK_SAMPLER_REDUCTION_MODE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SamplerReductionMode)
inline std::string to_string(SamplerReductionMode value)
{
	switch(value)
	{
	case SamplerReductionMode::e_WEIGHTED_AVERAGE: return "VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE";
	case SamplerReductionMode::e_MIN: return "VK_SAMPLER_REDUCTION_MODE_MIN";
	case SamplerReductionMode::e_MAX: return "VK_SAMPLER_REDUCTION_MODE_MAX";
	default: return "invalid";
	}
}

enum class TessellationDomainOrigin
{
	e_UPPER_LEFT = VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT,
	e_LOWER_LEFT = VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT,
	e_UPPER_LEFT_KHR = VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT_KHR,
	e_LOWER_LEFT_KHR = VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT_KHR,
	e_BEGIN_RANGE = VK_TESSELLATION_DOMAIN_ORIGIN_BEGIN_RANGE,
	e_END_RANGE = VK_TESSELLATION_DOMAIN_ORIGIN_END_RANGE,
	e_RANGE_SIZE = VK_TESSELLATION_DOMAIN_ORIGIN_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(TessellationDomainOrigin)
inline std::string to_string(TessellationDomainOrigin value)
{
	switch(value)
	{
	case TessellationDomainOrigin::e_UPPER_LEFT: return "VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT";
	case TessellationDomainOrigin::e_LOWER_LEFT: return "VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT";
	default: return "invalid";
	}
}

enum class SamplerYcbcrModelConversion
{
	e_RGB_IDENTITY = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY,
	e_YCBCR_IDENTITY = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY,
	e_YCBCR_709 = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
	e_YCBCR_601 = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601,
	e_YCBCR_2020 = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
	e_RGB_IDENTITY_KHR = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY_KHR,
	e_YCBCR_IDENTITY_KHR = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY_KHR,
	e_YCBCR_709_KHR = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709_KHR,
	e_YCBCR_601_KHR = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601_KHR,
	e_YCBCR_2020_KHR = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020_KHR,
	e_BEGIN_RANGE = VK_SAMPLER_YCBCR_MODEL_CONVERSION_BEGIN_RANGE,
	e_END_RANGE = VK_SAMPLER_YCBCR_MODEL_CONVERSION_END_RANGE,
	e_RANGE_SIZE = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SamplerYcbcrModelConversion)
inline std::string to_string(SamplerYcbcrModelConversion value)
{
	switch(value)
	{
	case SamplerYcbcrModelConversion::e_RGB_IDENTITY: return "VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY";
	case SamplerYcbcrModelConversion::e_YCBCR_IDENTITY: return "VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY";
	case SamplerYcbcrModelConversion::e_YCBCR_709: return "VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709";
	case SamplerYcbcrModelConversion::e_YCBCR_601: return "VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601";
	case SamplerYcbcrModelConversion::e_YCBCR_2020: return "VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020";
	default: return "invalid";
	}
}

enum class SamplerYcbcrRange
{
	e_ITU_FULL = VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
	e_ITU_NARROW = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
	e_ITU_FULL_KHR = VK_SAMPLER_YCBCR_RANGE_ITU_FULL_KHR,
	e_ITU_NARROW_KHR = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW_KHR,
	e_BEGIN_RANGE = VK_SAMPLER_YCBCR_RANGE_BEGIN_RANGE,
	e_END_RANGE = VK_SAMPLER_YCBCR_RANGE_END_RANGE,
	e_RANGE_SIZE = VK_SAMPLER_YCBCR_RANGE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(SamplerYcbcrRange)
inline std::string to_string(SamplerYcbcrRange value)
{
	switch(value)
	{
	case SamplerYcbcrRange::e_ITU_FULL: return "VK_SAMPLER_YCBCR_RANGE_ITU_FULL";
	case SamplerYcbcrRange::e_ITU_NARROW: return "VK_SAMPLER_YCBCR_RANGE_ITU_NARROW";
	default: return "invalid";
	}
}

enum class ChromaLocation
{
	e_COSITED_EVEN = VK_CHROMA_LOCATION_COSITED_EVEN,
	e_MIDPOINT = VK_CHROMA_LOCATION_MIDPOINT,
	e_COSITED_EVEN_KHR = VK_CHROMA_LOCATION_COSITED_EVEN_KHR,
	e_MIDPOINT_KHR = VK_CHROMA_LOCATION_MIDPOINT_KHR,
	e_BEGIN_RANGE = VK_CHROMA_LOCATION_BEGIN_RANGE,
	e_END_RANGE = VK_CHROMA_LOCATION_END_RANGE,
	e_RANGE_SIZE = VK_CHROMA_LOCATION_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ChromaLocation)
inline std::string to_string(ChromaLocation value)
{
	switch(value)
	{
	case ChromaLocation::e_COSITED_EVEN: return "VK_CHROMA_LOCATION_COSITED_EVEN";
	case ChromaLocation::e_MIDPOINT: return "VK_CHROMA_LOCATION_MIDPOINT";
	default: return "invalid";
	}
}

enum class BlendOverlapEXT
{
	e_UNCORRELATED_EXT = VK_BLEND_OVERLAP_UNCORRELATED_EXT,
	e_DISJOINT_EXT = VK_BLEND_OVERLAP_DISJOINT_EXT,
	e_CONJOINT_EXT = VK_BLEND_OVERLAP_CONJOINT_EXT,
	e_BEGIN_RANGE = VK_BLEND_OVERLAP_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_BLEND_OVERLAP_END_RANGE_EXT,
	e_RANGE_SIZE = VK_BLEND_OVERLAP_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(BlendOverlapEXT)
inline std::string to_string(BlendOverlapEXT value)
{
	switch(value)
	{
	case BlendOverlapEXT::e_UNCORRELATED_EXT: return "VK_BLEND_OVERLAP_UNCORRELATED_EXT";
	case BlendOverlapEXT::e_DISJOINT_EXT: return "VK_BLEND_OVERLAP_DISJOINT_EXT";
	case BlendOverlapEXT::e_CONJOINT_EXT: return "VK_BLEND_OVERLAP_CONJOINT_EXT";
	default: return "invalid";
	}
}

enum class CoverageModulationModeNV
{
	e_NONE_NV = VK_COVERAGE_MODULATION_MODE_NONE_NV,
	e_RGB_NV = VK_COVERAGE_MODULATION_MODE_RGB_NV,
	e_ALPHA_NV = VK_COVERAGE_MODULATION_MODE_ALPHA_NV,
	e_RGBA_NV = VK_COVERAGE_MODULATION_MODE_RGBA_NV,
	e_BEGIN_RANGE = VK_COVERAGE_MODULATION_MODE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_COVERAGE_MODULATION_MODE_END_RANGE_NV,
	e_RANGE_SIZE = VK_COVERAGE_MODULATION_MODE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CoverageModulationModeNV)
inline std::string to_string(CoverageModulationModeNV value)
{
	switch(value)
	{
	case CoverageModulationModeNV::e_NONE_NV: return "VK_COVERAGE_MODULATION_MODE_NONE_NV";
	case CoverageModulationModeNV::e_RGB_NV: return "VK_COVERAGE_MODULATION_MODE_RGB_NV";
	case CoverageModulationModeNV::e_ALPHA_NV: return "VK_COVERAGE_MODULATION_MODE_ALPHA_NV";
	case CoverageModulationModeNV::e_RGBA_NV: return "VK_COVERAGE_MODULATION_MODE_RGBA_NV";
	default: return "invalid";
	}
}

enum class CoverageReductionModeNV
{
	e_MERGE_NV = VK_COVERAGE_REDUCTION_MODE_MERGE_NV,
	e_TRUNCATE_NV = VK_COVERAGE_REDUCTION_MODE_TRUNCATE_NV,
	e_BEGIN_RANGE = VK_COVERAGE_REDUCTION_MODE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_COVERAGE_REDUCTION_MODE_END_RANGE_NV,
	e_RANGE_SIZE = VK_COVERAGE_REDUCTION_MODE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CoverageReductionModeNV)
inline std::string to_string(CoverageReductionModeNV value)
{
	switch(value)
	{
	case CoverageReductionModeNV::e_MERGE_NV: return "VK_COVERAGE_REDUCTION_MODE_MERGE_NV";
	case CoverageReductionModeNV::e_TRUNCATE_NV: return "VK_COVERAGE_REDUCTION_MODE_TRUNCATE_NV";
	default: return "invalid";
	}
}

enum class ValidationCacheHeaderVersionEXT
{
	e_ONE_EXT = VK_VALIDATION_CACHE_HEADER_VERSION_ONE_EXT,
	e_BEGIN_RANGE = VK_VALIDATION_CACHE_HEADER_VERSION_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_VALIDATION_CACHE_HEADER_VERSION_END_RANGE_EXT,
	e_RANGE_SIZE = VK_VALIDATION_CACHE_HEADER_VERSION_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ValidationCacheHeaderVersionEXT)
inline std::string to_string(ValidationCacheHeaderVersionEXT value)
{
	switch(value)
	{
	case ValidationCacheHeaderVersionEXT::e_ONE_EXT: return "VK_VALIDATION_CACHE_HEADER_VERSION_ONE_EXT";
	default: return "invalid";
	}
}

enum class ShaderInfoTypeAMD
{
	e_STATISTICS_AMD = VK_SHADER_INFO_TYPE_STATISTICS_AMD,
	e_BINARY_AMD = VK_SHADER_INFO_TYPE_BINARY_AMD,
	e_DISASSEMBLY_AMD = VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD,
	e_BEGIN_RANGE = VK_SHADER_INFO_TYPE_BEGIN_RANGE_AMD,
	e_END_RANGE = VK_SHADER_INFO_TYPE_END_RANGE_AMD,
	e_RANGE_SIZE = VK_SHADER_INFO_TYPE_RANGE_SIZE_AMD,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ShaderInfoTypeAMD)
inline std::string to_string(ShaderInfoTypeAMD value)
{
	switch(value)
	{
	case ShaderInfoTypeAMD::e_STATISTICS_AMD: return "VK_SHADER_INFO_TYPE_STATISTICS_AMD";
	case ShaderInfoTypeAMD::e_BINARY_AMD: return "VK_SHADER_INFO_TYPE_BINARY_AMD";
	case ShaderInfoTypeAMD::e_DISASSEMBLY_AMD: return "VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD";
	default: return "invalid";
	}
}

enum class QueueGlobalPriorityEXT
{
	e_LOW_EXT = VK_QUEUE_GLOBAL_PRIORITY_LOW_EXT,
	e_MEDIUM_EXT = VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT,
	e_HIGH_EXT = VK_QUEUE_GLOBAL_PRIORITY_HIGH_EXT,
	e_REALTIME_EXT = VK_QUEUE_GLOBAL_PRIORITY_REALTIME_EXT,
	e_BEGIN_RANGE = VK_QUEUE_GLOBAL_PRIORITY_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_QUEUE_GLOBAL_PRIORITY_END_RANGE_EXT,
	e_RANGE_SIZE = VK_QUEUE_GLOBAL_PRIORITY_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(QueueGlobalPriorityEXT)
inline std::string to_string(QueueGlobalPriorityEXT value)
{
	switch(value)
	{
	case QueueGlobalPriorityEXT::e_LOW_EXT: return "VK_QUEUE_GLOBAL_PRIORITY_LOW_EXT";
	case QueueGlobalPriorityEXT::e_MEDIUM_EXT: return "VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT";
	case QueueGlobalPriorityEXT::e_HIGH_EXT: return "VK_QUEUE_GLOBAL_PRIORITY_HIGH_EXT";
	case QueueGlobalPriorityEXT::e_REALTIME_EXT: return "VK_QUEUE_GLOBAL_PRIORITY_REALTIME_EXT";
	default: return "invalid";
	}
}

enum class ConservativeRasterizationModeEXT
{
	e_DISABLED_EXT = VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT,
	e_OVERESTIMATE_EXT = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT,
	e_UNDERESTIMATE_EXT = VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT,
	e_BEGIN_RANGE = VK_CONSERVATIVE_RASTERIZATION_MODE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_CONSERVATIVE_RASTERIZATION_MODE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_CONSERVATIVE_RASTERIZATION_MODE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ConservativeRasterizationModeEXT)
inline std::string to_string(ConservativeRasterizationModeEXT value)
{
	switch(value)
	{
	case ConservativeRasterizationModeEXT::e_DISABLED_EXT: return "VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT";
	case ConservativeRasterizationModeEXT::e_OVERESTIMATE_EXT: return "VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT";
	case ConservativeRasterizationModeEXT::e_UNDERESTIMATE_EXT: return "VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT";
	default: return "invalid";
	}
}

enum class VendorId
{
	e_VIV = VK_VENDOR_ID_VIV,
	e_VSI = VK_VENDOR_ID_VSI,
	e_KAZAN = VK_VENDOR_ID_KAZAN,
	e_BEGIN_RANGE = VK_VENDOR_ID_BEGIN_RANGE,
	e_END_RANGE = VK_VENDOR_ID_END_RANGE,
	e_RANGE_SIZE = VK_VENDOR_ID_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(VendorId)
inline std::string to_string(VendorId value)
{
	switch(value)
	{
	case VendorId::e_VIV: return "VK_VENDOR_ID_VIV";
	case VendorId::e_VSI: return "VK_VENDOR_ID_VSI";
	case VendorId::e_KAZAN: return "VK_VENDOR_ID_KAZAN";
	default: return "invalid";
	}
}

enum class DriverId
{
	e_AMD_PROPRIETARY = VK_DRIVER_ID_AMD_PROPRIETARY,
	e_AMD_OPEN_SOURCE = VK_DRIVER_ID_AMD_OPEN_SOURCE,
	e_MESA_RADV = VK_DRIVER_ID_MESA_RADV,
	e_NVIDIA_PROPRIETARY = VK_DRIVER_ID_NVIDIA_PROPRIETARY,
	e_INTEL_PROPRIETARY_WINDOWS = VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS,
	e_INTEL_OPEN_SOURCE_MESA = VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA,
	e_IMAGINATION_PROPRIETARY = VK_DRIVER_ID_IMAGINATION_PROPRIETARY,
	e_QUALCOMM_PROPRIETARY = VK_DRIVER_ID_QUALCOMM_PROPRIETARY,
	e_ARM_PROPRIETARY = VK_DRIVER_ID_ARM_PROPRIETARY,
	e_GOOGLE_SWIFTSHADER = VK_DRIVER_ID_GOOGLE_SWIFTSHADER,
	e_GGP_PROPRIETARY = VK_DRIVER_ID_GGP_PROPRIETARY,
	e_BROADCOM_PROPRIETARY = VK_DRIVER_ID_BROADCOM_PROPRIETARY,
	e_AMD_PROPRIETARY_KHR = VK_DRIVER_ID_AMD_PROPRIETARY_KHR,
	e_AMD_OPEN_SOURCE_KHR = VK_DRIVER_ID_AMD_OPEN_SOURCE_KHR,
	e_MESA_RADV_KHR = VK_DRIVER_ID_MESA_RADV_KHR,
	e_NVIDIA_PROPRIETARY_KHR = VK_DRIVER_ID_NVIDIA_PROPRIETARY_KHR,
	e_INTEL_PROPRIETARY_WINDOWS_KHR = VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS_KHR,
	e_INTEL_OPEN_SOURCE_MESA_KHR = VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA_KHR,
	e_IMAGINATION_PROPRIETARY_KHR = VK_DRIVER_ID_IMAGINATION_PROPRIETARY_KHR,
	e_QUALCOMM_PROPRIETARY_KHR = VK_DRIVER_ID_QUALCOMM_PROPRIETARY_KHR,
	e_ARM_PROPRIETARY_KHR = VK_DRIVER_ID_ARM_PROPRIETARY_KHR,
	e_GOOGLE_SWIFTSHADER_KHR = VK_DRIVER_ID_GOOGLE_SWIFTSHADER_KHR,
	e_GGP_PROPRIETARY_KHR = VK_DRIVER_ID_GGP_PROPRIETARY_KHR,
	e_BROADCOM_PROPRIETARY_KHR = VK_DRIVER_ID_BROADCOM_PROPRIETARY_KHR,
	e_BEGIN_RANGE = VK_DRIVER_ID_BEGIN_RANGE,
	e_END_RANGE = VK_DRIVER_ID_END_RANGE,
	e_RANGE_SIZE = VK_DRIVER_ID_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(DriverId)
inline std::string to_string(DriverId value)
{
	switch(value)
	{
	case DriverId::e_AMD_PROPRIETARY: return "VK_DRIVER_ID_AMD_PROPRIETARY";
	case DriverId::e_AMD_OPEN_SOURCE: return "VK_DRIVER_ID_AMD_OPEN_SOURCE";
	case DriverId::e_MESA_RADV: return "VK_DRIVER_ID_MESA_RADV";
	case DriverId::e_NVIDIA_PROPRIETARY: return "VK_DRIVER_ID_NVIDIA_PROPRIETARY";
	case DriverId::e_INTEL_PROPRIETARY_WINDOWS: return "VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS";
	case DriverId::e_INTEL_OPEN_SOURCE_MESA: return "VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA";
	case DriverId::e_IMAGINATION_PROPRIETARY: return "VK_DRIVER_ID_IMAGINATION_PROPRIETARY";
	case DriverId::e_QUALCOMM_PROPRIETARY: return "VK_DRIVER_ID_QUALCOMM_PROPRIETARY";
	case DriverId::e_ARM_PROPRIETARY: return "VK_DRIVER_ID_ARM_PROPRIETARY";
	case DriverId::e_GOOGLE_SWIFTSHADER: return "VK_DRIVER_ID_GOOGLE_SWIFTSHADER";
	case DriverId::e_GGP_PROPRIETARY: return "VK_DRIVER_ID_GGP_PROPRIETARY";
	case DriverId::e_BROADCOM_PROPRIETARY: return "VK_DRIVER_ID_BROADCOM_PROPRIETARY";
	default: return "invalid";
	}
}

enum class ShadingRatePaletteEntryNV
{
	e_NO_INVOCATIONS_NV = VK_SHADING_RATE_PALETTE_ENTRY_NO_INVOCATIONS_NV,
	e_16_INVOCATIONS_PER_PIXEL_NV = VK_SHADING_RATE_PALETTE_ENTRY_16_INVOCATIONS_PER_PIXEL_NV,
	e_8_INVOCATIONS_PER_PIXEL_NV = VK_SHADING_RATE_PALETTE_ENTRY_8_INVOCATIONS_PER_PIXEL_NV,
	e_4_INVOCATIONS_PER_PIXEL_NV = VK_SHADING_RATE_PALETTE_ENTRY_4_INVOCATIONS_PER_PIXEL_NV,
	e_2_INVOCATIONS_PER_PIXEL_NV = VK_SHADING_RATE_PALETTE_ENTRY_2_INVOCATIONS_PER_PIXEL_NV,
	e_1_INVOCATION_PER_PIXEL_NV = VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV,
	e_1_INVOCATION_PER_2X1_PIXELS_NV = VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV,
	e_1_INVOCATION_PER_1X2_PIXELS_NV = VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV,
	e_1_INVOCATION_PER_2X2_PIXELS_NV = VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV,
	e_1_INVOCATION_PER_4X2_PIXELS_NV = VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV,
	e_1_INVOCATION_PER_2X4_PIXELS_NV = VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV,
	e_1_INVOCATION_PER_4X4_PIXELS_NV = VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV,
	e_BEGIN_RANGE = VK_SHADING_RATE_PALETTE_ENTRY_BEGIN_RANGE_NV,
	e_END_RANGE = VK_SHADING_RATE_PALETTE_ENTRY_END_RANGE_NV,
	e_RANGE_SIZE = VK_SHADING_RATE_PALETTE_ENTRY_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ShadingRatePaletteEntryNV)
inline std::string to_string(ShadingRatePaletteEntryNV value)
{
	switch(value)
	{
	case ShadingRatePaletteEntryNV::e_NO_INVOCATIONS_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_NO_INVOCATIONS_NV";
	case ShadingRatePaletteEntryNV::e_16_INVOCATIONS_PER_PIXEL_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_16_INVOCATIONS_PER_PIXEL_NV";
	case ShadingRatePaletteEntryNV::e_8_INVOCATIONS_PER_PIXEL_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_8_INVOCATIONS_PER_PIXEL_NV";
	case ShadingRatePaletteEntryNV::e_4_INVOCATIONS_PER_PIXEL_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_4_INVOCATIONS_PER_PIXEL_NV";
	case ShadingRatePaletteEntryNV::e_2_INVOCATIONS_PER_PIXEL_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_2_INVOCATIONS_PER_PIXEL_NV";
	case ShadingRatePaletteEntryNV::e_1_INVOCATION_PER_PIXEL_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV";
	case ShadingRatePaletteEntryNV::e_1_INVOCATION_PER_2X1_PIXELS_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV";
	case ShadingRatePaletteEntryNV::e_1_INVOCATION_PER_1X2_PIXELS_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV";
	case ShadingRatePaletteEntryNV::e_1_INVOCATION_PER_2X2_PIXELS_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV";
	case ShadingRatePaletteEntryNV::e_1_INVOCATION_PER_4X2_PIXELS_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV";
	case ShadingRatePaletteEntryNV::e_1_INVOCATION_PER_2X4_PIXELS_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV";
	case ShadingRatePaletteEntryNV::e_1_INVOCATION_PER_4X4_PIXELS_NV: return "VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV";
	default: return "invalid";
	}
}

enum class CoarseSampleOrderTypeNV
{
	e_DEFAULT_NV = VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV,
	e_CUSTOM_NV = VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV,
	e_PIXEL_MAJOR_NV = VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV,
	e_SAMPLE_MAJOR_NV = VK_COARSE_SAMPLE_ORDER_TYPE_SAMPLE_MAJOR_NV,
	e_BEGIN_RANGE = VK_COARSE_SAMPLE_ORDER_TYPE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_COARSE_SAMPLE_ORDER_TYPE_END_RANGE_NV,
	e_RANGE_SIZE = VK_COARSE_SAMPLE_ORDER_TYPE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CoarseSampleOrderTypeNV)
inline std::string to_string(CoarseSampleOrderTypeNV value)
{
	switch(value)
	{
	case CoarseSampleOrderTypeNV::e_DEFAULT_NV: return "VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV";
	case CoarseSampleOrderTypeNV::e_CUSTOM_NV: return "VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV";
	case CoarseSampleOrderTypeNV::e_PIXEL_MAJOR_NV: return "VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV";
	case CoarseSampleOrderTypeNV::e_SAMPLE_MAJOR_NV: return "VK_COARSE_SAMPLE_ORDER_TYPE_SAMPLE_MAJOR_NV";
	default: return "invalid";
	}
}

enum class CopyAccelerationStructureModeNV
{
	e_CLONE_NV = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV,
	e_COMPACT_NV = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV,
	e_BEGIN_RANGE = VK_COPY_ACCELERATION_STRUCTURE_MODE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_COPY_ACCELERATION_STRUCTURE_MODE_END_RANGE_NV,
	e_RANGE_SIZE = VK_COPY_ACCELERATION_STRUCTURE_MODE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(CopyAccelerationStructureModeNV)
inline std::string to_string(CopyAccelerationStructureModeNV value)
{
	switch(value)
	{
	case CopyAccelerationStructureModeNV::e_CLONE_NV: return "VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV";
	case CopyAccelerationStructureModeNV::e_COMPACT_NV: return "VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV";
	default: return "invalid";
	}
}

enum class AccelerationStructureTypeNV
{
	e_TOP_LEVEL_NV = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV,
	e_BOTTOM_LEVEL_NV = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV,
	e_BEGIN_RANGE = VK_ACCELERATION_STRUCTURE_TYPE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_ACCELERATION_STRUCTURE_TYPE_END_RANGE_NV,
	e_RANGE_SIZE = VK_ACCELERATION_STRUCTURE_TYPE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(AccelerationStructureTypeNV)
inline std::string to_string(AccelerationStructureTypeNV value)
{
	switch(value)
	{
	case AccelerationStructureTypeNV::e_TOP_LEVEL_NV: return "VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV";
	case AccelerationStructureTypeNV::e_BOTTOM_LEVEL_NV: return "VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV";
	default: return "invalid";
	}
}

enum class GeometryTypeNV
{
	e_TRIANGLES_NV = VK_GEOMETRY_TYPE_TRIANGLES_NV,
	e_AABBS_NV = VK_GEOMETRY_TYPE_AABBS_NV,
	e_BEGIN_RANGE = VK_GEOMETRY_TYPE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_GEOMETRY_TYPE_END_RANGE_NV,
	e_RANGE_SIZE = VK_GEOMETRY_TYPE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(GeometryTypeNV)
inline std::string to_string(GeometryTypeNV value)
{
	switch(value)
	{
	case GeometryTypeNV::e_TRIANGLES_NV: return "VK_GEOMETRY_TYPE_TRIANGLES_NV";
	case GeometryTypeNV::e_AABBS_NV: return "VK_GEOMETRY_TYPE_AABBS_NV";
	default: return "invalid";
	}
}

enum class AccelerationStructureMemoryRequirementsTypeNV
{
	e_OBJECT_NV = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV,
	e_BUILD_SCRATCH_NV = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV,
	e_UPDATE_SCRATCH_NV = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV,
	e_BEGIN_RANGE = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_END_RANGE_NV,
	e_RANGE_SIZE = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(AccelerationStructureMemoryRequirementsTypeNV)
inline std::string to_string(AccelerationStructureMemoryRequirementsTypeNV value)
{
	switch(value)
	{
	case AccelerationStructureMemoryRequirementsTypeNV::e_OBJECT_NV: return "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV";
	case AccelerationStructureMemoryRequirementsTypeNV::e_BUILD_SCRATCH_NV: return "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV";
	case AccelerationStructureMemoryRequirementsTypeNV::e_UPDATE_SCRATCH_NV: return "VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV";
	default: return "invalid";
	}
}

enum class RayTracingShaderGroupTypeNV
{
	e_GENERAL_NV = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV,
	e_TRIANGLES_HIT_GROUP_NV = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV,
	e_PROCEDURAL_HIT_GROUP_NV = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV,
	e_BEGIN_RANGE = VK_RAY_TRACING_SHADER_GROUP_TYPE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_RAY_TRACING_SHADER_GROUP_TYPE_END_RANGE_NV,
	e_RANGE_SIZE = VK_RAY_TRACING_SHADER_GROUP_TYPE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(RayTracingShaderGroupTypeNV)
inline std::string to_string(RayTracingShaderGroupTypeNV value)
{
	switch(value)
	{
	case RayTracingShaderGroupTypeNV::e_GENERAL_NV: return "VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV";
	case RayTracingShaderGroupTypeNV::e_TRIANGLES_HIT_GROUP_NV: return "VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV";
	case RayTracingShaderGroupTypeNV::e_PROCEDURAL_HIT_GROUP_NV: return "VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV";
	default: return "invalid";
	}
}

enum class MemoryOverallocationBehaviorAMD
{
	e_DEFAULT_AMD = VK_MEMORY_OVERALLOCATION_BEHAVIOR_DEFAULT_AMD,
	e_ALLOWED_AMD = VK_MEMORY_OVERALLOCATION_BEHAVIOR_ALLOWED_AMD,
	e_DISALLOWED_AMD = VK_MEMORY_OVERALLOCATION_BEHAVIOR_DISALLOWED_AMD,
	e_BEGIN_RANGE = VK_MEMORY_OVERALLOCATION_BEHAVIOR_BEGIN_RANGE_AMD,
	e_END_RANGE = VK_MEMORY_OVERALLOCATION_BEHAVIOR_END_RANGE_AMD,
	e_RANGE_SIZE = VK_MEMORY_OVERALLOCATION_BEHAVIOR_RANGE_SIZE_AMD,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(MemoryOverallocationBehaviorAMD)
inline std::string to_string(MemoryOverallocationBehaviorAMD value)
{
	switch(value)
	{
	case MemoryOverallocationBehaviorAMD::e_DEFAULT_AMD: return "VK_MEMORY_OVERALLOCATION_BEHAVIOR_DEFAULT_AMD";
	case MemoryOverallocationBehaviorAMD::e_ALLOWED_AMD: return "VK_MEMORY_OVERALLOCATION_BEHAVIOR_ALLOWED_AMD";
	case MemoryOverallocationBehaviorAMD::e_DISALLOWED_AMD: return "VK_MEMORY_OVERALLOCATION_BEHAVIOR_DISALLOWED_AMD";
	default: return "invalid";
	}
}

enum class ScopeNV
{
	e_DEVICE_NV = VK_SCOPE_DEVICE_NV,
	e_WORKGROUP_NV = VK_SCOPE_WORKGROUP_NV,
	e_SUBGROUP_NV = VK_SCOPE_SUBGROUP_NV,
	e_QUEUE_FAMILY_NV = VK_SCOPE_QUEUE_FAMILY_NV,
	e_BEGIN_RANGE = VK_SCOPE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_SCOPE_END_RANGE_NV,
	e_RANGE_SIZE = VK_SCOPE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ScopeNV)
inline std::string to_string(ScopeNV value)
{
	switch(value)
	{
	case ScopeNV::e_DEVICE_NV: return "VK_SCOPE_DEVICE_NV";
	case ScopeNV::e_WORKGROUP_NV: return "VK_SCOPE_WORKGROUP_NV";
	case ScopeNV::e_SUBGROUP_NV: return "VK_SCOPE_SUBGROUP_NV";
	case ScopeNV::e_QUEUE_FAMILY_NV: return "VK_SCOPE_QUEUE_FAMILY_NV";
	default: return "invalid";
	}
}

enum class ComponentTypeNV
{
	e_FLOAT16_NV = VK_COMPONENT_TYPE_FLOAT16_NV,
	e_FLOAT32_NV = VK_COMPONENT_TYPE_FLOAT32_NV,
	e_FLOAT64_NV = VK_COMPONENT_TYPE_FLOAT64_NV,
	e_SINT8_NV = VK_COMPONENT_TYPE_SINT8_NV,
	e_SINT16_NV = VK_COMPONENT_TYPE_SINT16_NV,
	e_SINT32_NV = VK_COMPONENT_TYPE_SINT32_NV,
	e_SINT64_NV = VK_COMPONENT_TYPE_SINT64_NV,
	e_UINT8_NV = VK_COMPONENT_TYPE_UINT8_NV,
	e_UINT16_NV = VK_COMPONENT_TYPE_UINT16_NV,
	e_UINT32_NV = VK_COMPONENT_TYPE_UINT32_NV,
	e_UINT64_NV = VK_COMPONENT_TYPE_UINT64_NV,
	e_BEGIN_RANGE = VK_COMPONENT_TYPE_BEGIN_RANGE_NV,
	e_END_RANGE = VK_COMPONENT_TYPE_END_RANGE_NV,
	e_RANGE_SIZE = VK_COMPONENT_TYPE_RANGE_SIZE_NV,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ComponentTypeNV)
inline std::string to_string(ComponentTypeNV value)
{
	switch(value)
	{
	case ComponentTypeNV::e_FLOAT16_NV: return "VK_COMPONENT_TYPE_FLOAT16_NV";
	case ComponentTypeNV::e_FLOAT32_NV: return "VK_COMPONENT_TYPE_FLOAT32_NV";
	case ComponentTypeNV::e_FLOAT64_NV: return "VK_COMPONENT_TYPE_FLOAT64_NV";
	case ComponentTypeNV::e_SINT8_NV: return "VK_COMPONENT_TYPE_SINT8_NV";
	case ComponentTypeNV::e_SINT16_NV: return "VK_COMPONENT_TYPE_SINT16_NV";
	case ComponentTypeNV::e_SINT32_NV: return "VK_COMPONENT_TYPE_SINT32_NV";
	case ComponentTypeNV::e_SINT64_NV: return "VK_COMPONENT_TYPE_SINT64_NV";
	case ComponentTypeNV::e_UINT8_NV: return "VK_COMPONENT_TYPE_UINT8_NV";
	case ComponentTypeNV::e_UINT16_NV: return "VK_COMPONENT_TYPE_UINT16_NV";
	case ComponentTypeNV::e_UINT32_NV: return "VK_COMPONENT_TYPE_UINT32_NV";
	case ComponentTypeNV::e_UINT64_NV: return "VK_COMPONENT_TYPE_UINT64_NV";
	default: return "invalid";
	}
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
enum class FullScreenExclusiveEXT
{
	e_DEFAULT_EXT = VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT,
	e_ALLOWED_EXT = VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT,
	e_DISALLOWED_EXT = VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT,
	e_APPLICATION_CONTROLLED_EXT = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT,
	e_BEGIN_RANGE = VK_FULL_SCREEN_EXCLUSIVE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_FULL_SCREEN_EXCLUSIVE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_FULL_SCREEN_EXCLUSIVE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(FullScreenExclusiveEXT)
inline std::string to_string(FullScreenExclusiveEXT value)
{
	switch(value)
	{
	case FullScreenExclusiveEXT::e_DEFAULT_EXT: return "VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT";
	case FullScreenExclusiveEXT::e_ALLOWED_EXT: return "VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT";
	case FullScreenExclusiveEXT::e_DISALLOWED_EXT: return "VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT";
	case FullScreenExclusiveEXT::e_APPLICATION_CONTROLLED_EXT: return "VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT";
	default: return "invalid";
	}
}
#endif // VK_USE_PLATFORM_WIN32_KHR

enum class PerformanceCounterScopeKHR
{
	e_COMMAND_BUFFER_KHR = VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_BUFFER_KHR,
	e_RENDER_PASS_KHR = VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR,
	e_COMMAND_KHR = VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR,
	e_BEGIN_RANGE = VK_PERFORMANCE_COUNTER_SCOPE_BEGIN_RANGE_KHR,
	e_END_RANGE = VK_PERFORMANCE_COUNTER_SCOPE_END_RANGE_KHR,
	e_RANGE_SIZE = VK_PERFORMANCE_COUNTER_SCOPE_RANGE_SIZE_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PerformanceCounterScopeKHR)
inline std::string to_string(PerformanceCounterScopeKHR value)
{
	switch(value)
	{
	case PerformanceCounterScopeKHR::e_COMMAND_BUFFER_KHR: return "VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_BUFFER_KHR";
	case PerformanceCounterScopeKHR::e_RENDER_PASS_KHR: return "VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR";
	case PerformanceCounterScopeKHR::e_COMMAND_KHR: return "VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR";
	default: return "invalid";
	}
}

enum class PerformanceCounterUnitKHR
{
	e_GENERIC_KHR = VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR,
	e_PERCENTAGE_KHR = VK_PERFORMANCE_COUNTER_UNIT_PERCENTAGE_KHR,
	e_NANOSECONDS_KHR = VK_PERFORMANCE_COUNTER_UNIT_NANOSECONDS_KHR,
	e_BYTES_KHR = VK_PERFORMANCE_COUNTER_UNIT_BYTES_KHR,
	e_BYTES_PER_SECOND_KHR = VK_PERFORMANCE_COUNTER_UNIT_BYTES_PER_SECOND_KHR,
	e_KELVIN_KHR = VK_PERFORMANCE_COUNTER_UNIT_KELVIN_KHR,
	e_WATTS_KHR = VK_PERFORMANCE_COUNTER_UNIT_WATTS_KHR,
	e_VOLTS_KHR = VK_PERFORMANCE_COUNTER_UNIT_VOLTS_KHR,
	e_AMPS_KHR = VK_PERFORMANCE_COUNTER_UNIT_AMPS_KHR,
	e_HERTZ_KHR = VK_PERFORMANCE_COUNTER_UNIT_HERTZ_KHR,
	e_CYCLES_KHR = VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR,
	e_BEGIN_RANGE = VK_PERFORMANCE_COUNTER_UNIT_BEGIN_RANGE_KHR,
	e_END_RANGE = VK_PERFORMANCE_COUNTER_UNIT_END_RANGE_KHR,
	e_RANGE_SIZE = VK_PERFORMANCE_COUNTER_UNIT_RANGE_SIZE_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PerformanceCounterUnitKHR)
inline std::string to_string(PerformanceCounterUnitKHR value)
{
	switch(value)
	{
	case PerformanceCounterUnitKHR::e_GENERIC_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_GENERIC_KHR";
	case PerformanceCounterUnitKHR::e_PERCENTAGE_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_PERCENTAGE_KHR";
	case PerformanceCounterUnitKHR::e_NANOSECONDS_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_NANOSECONDS_KHR";
	case PerformanceCounterUnitKHR::e_BYTES_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_BYTES_KHR";
	case PerformanceCounterUnitKHR::e_BYTES_PER_SECOND_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_BYTES_PER_SECOND_KHR";
	case PerformanceCounterUnitKHR::e_KELVIN_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_KELVIN_KHR";
	case PerformanceCounterUnitKHR::e_WATTS_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_WATTS_KHR";
	case PerformanceCounterUnitKHR::e_VOLTS_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_VOLTS_KHR";
	case PerformanceCounterUnitKHR::e_AMPS_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_AMPS_KHR";
	case PerformanceCounterUnitKHR::e_HERTZ_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_HERTZ_KHR";
	case PerformanceCounterUnitKHR::e_CYCLES_KHR: return "VK_PERFORMANCE_COUNTER_UNIT_CYCLES_KHR";
	default: return "invalid";
	}
}

enum class PerformanceCounterStorageKHR
{
	e_INT32_KHR = VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR,
	e_INT64_KHR = VK_PERFORMANCE_COUNTER_STORAGE_INT64_KHR,
	e_UINT32_KHR = VK_PERFORMANCE_COUNTER_STORAGE_UINT32_KHR,
	e_UINT64_KHR = VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR,
	e_FLOAT32_KHR = VK_PERFORMANCE_COUNTER_STORAGE_FLOAT32_KHR,
	e_FLOAT64_KHR = VK_PERFORMANCE_COUNTER_STORAGE_FLOAT64_KHR,
	e_BEGIN_RANGE = VK_PERFORMANCE_COUNTER_STORAGE_BEGIN_RANGE_KHR,
	e_END_RANGE = VK_PERFORMANCE_COUNTER_STORAGE_END_RANGE_KHR,
	e_RANGE_SIZE = VK_PERFORMANCE_COUNTER_STORAGE_RANGE_SIZE_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PerformanceCounterStorageKHR)
inline std::string to_string(PerformanceCounterStorageKHR value)
{
	switch(value)
	{
	case PerformanceCounterStorageKHR::e_INT32_KHR: return "VK_PERFORMANCE_COUNTER_STORAGE_INT32_KHR";
	case PerformanceCounterStorageKHR::e_INT64_KHR: return "VK_PERFORMANCE_COUNTER_STORAGE_INT64_KHR";
	case PerformanceCounterStorageKHR::e_UINT32_KHR: return "VK_PERFORMANCE_COUNTER_STORAGE_UINT32_KHR";
	case PerformanceCounterStorageKHR::e_UINT64_KHR: return "VK_PERFORMANCE_COUNTER_STORAGE_UINT64_KHR";
	case PerformanceCounterStorageKHR::e_FLOAT32_KHR: return "VK_PERFORMANCE_COUNTER_STORAGE_FLOAT32_KHR";
	case PerformanceCounterStorageKHR::e_FLOAT64_KHR: return "VK_PERFORMANCE_COUNTER_STORAGE_FLOAT64_KHR";
	default: return "invalid";
	}
}

enum class PerformanceConfigurationTypeINTEL
{
	e_COMMAND_QUEUE_METRICS_DISCOVERY_ACTIVATED_INTEL = VK_PERFORMANCE_CONFIGURATION_TYPE_COMMAND_QUEUE_METRICS_DISCOVERY_ACTIVATED_INTEL,
	e_BEGIN_RANGE = VK_PERFORMANCE_CONFIGURATION_TYPE_BEGIN_RANGE_INTEL,
	e_END_RANGE = VK_PERFORMANCE_CONFIGURATION_TYPE_END_RANGE_INTEL,
	e_RANGE_SIZE = VK_PERFORMANCE_CONFIGURATION_TYPE_RANGE_SIZE_INTEL,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PerformanceConfigurationTypeINTEL)
inline std::string to_string(PerformanceConfigurationTypeINTEL value)
{
	switch(value)
	{
	case PerformanceConfigurationTypeINTEL::e_COMMAND_QUEUE_METRICS_DISCOVERY_ACTIVATED_INTEL: return "VK_PERFORMANCE_CONFIGURATION_TYPE_COMMAND_QUEUE_METRICS_DISCOVERY_ACTIVATED_INTEL";
	default: return "invalid";
	}
}

enum class QueryPoolSamplingModeINTEL
{
	e_MANUAL_INTEL = VK_QUERY_POOL_SAMPLING_MODE_MANUAL_INTEL,
	e_BEGIN_RANGE = VK_QUERY_POOL_SAMPLING_MODE_BEGIN_RANGE_INTEL,
	e_END_RANGE = VK_QUERY_POOL_SAMPLING_MODE_END_RANGE_INTEL,
	e_RANGE_SIZE = VK_QUERY_POOL_SAMPLING_MODE_RANGE_SIZE_INTEL,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(QueryPoolSamplingModeINTEL)
inline std::string to_string(QueryPoolSamplingModeINTEL value)
{
	switch(value)
	{
	case QueryPoolSamplingModeINTEL::e_MANUAL_INTEL: return "VK_QUERY_POOL_SAMPLING_MODE_MANUAL_INTEL";
	default: return "invalid";
	}
}

enum class PerformanceOverrideTypeINTEL
{
	e_NULL_HARDWARE_INTEL = VK_PERFORMANCE_OVERRIDE_TYPE_NULL_HARDWARE_INTEL,
	e_FLUSH_GPU_CACHES_INTEL = VK_PERFORMANCE_OVERRIDE_TYPE_FLUSH_GPU_CACHES_INTEL,
	e_BEGIN_RANGE = VK_PERFORMANCE_OVERRIDE_TYPE_BEGIN_RANGE_INTEL,
	e_END_RANGE = VK_PERFORMANCE_OVERRIDE_TYPE_END_RANGE_INTEL,
	e_RANGE_SIZE = VK_PERFORMANCE_OVERRIDE_TYPE_RANGE_SIZE_INTEL,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PerformanceOverrideTypeINTEL)
inline std::string to_string(PerformanceOverrideTypeINTEL value)
{
	switch(value)
	{
	case PerformanceOverrideTypeINTEL::e_NULL_HARDWARE_INTEL: return "VK_PERFORMANCE_OVERRIDE_TYPE_NULL_HARDWARE_INTEL";
	case PerformanceOverrideTypeINTEL::e_FLUSH_GPU_CACHES_INTEL: return "VK_PERFORMANCE_OVERRIDE_TYPE_FLUSH_GPU_CACHES_INTEL";
	default: return "invalid";
	}
}

enum class PerformanceParameterTypeINTEL
{
	e_HW_COUNTERS_SUPPORTED_INTEL = VK_PERFORMANCE_PARAMETER_TYPE_HW_COUNTERS_SUPPORTED_INTEL,
	e_STREAM_MARKER_VALID_BITS_INTEL = VK_PERFORMANCE_PARAMETER_TYPE_STREAM_MARKER_VALID_BITS_INTEL,
	e_BEGIN_RANGE = VK_PERFORMANCE_PARAMETER_TYPE_BEGIN_RANGE_INTEL,
	e_END_RANGE = VK_PERFORMANCE_PARAMETER_TYPE_END_RANGE_INTEL,
	e_RANGE_SIZE = VK_PERFORMANCE_PARAMETER_TYPE_RANGE_SIZE_INTEL,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PerformanceParameterTypeINTEL)
inline std::string to_string(PerformanceParameterTypeINTEL value)
{
	switch(value)
	{
	case PerformanceParameterTypeINTEL::e_HW_COUNTERS_SUPPORTED_INTEL: return "VK_PERFORMANCE_PARAMETER_TYPE_HW_COUNTERS_SUPPORTED_INTEL";
	case PerformanceParameterTypeINTEL::e_STREAM_MARKER_VALID_BITS_INTEL: return "VK_PERFORMANCE_PARAMETER_TYPE_STREAM_MARKER_VALID_BITS_INTEL";
	default: return "invalid";
	}
}

enum class PerformanceValueTypeINTEL
{
	e_UINT32_INTEL = VK_PERFORMANCE_VALUE_TYPE_UINT32_INTEL,
	e_UINT64_INTEL = VK_PERFORMANCE_VALUE_TYPE_UINT64_INTEL,
	e_FLOAT_INTEL = VK_PERFORMANCE_VALUE_TYPE_FLOAT_INTEL,
	e_BOOL_INTEL = VK_PERFORMANCE_VALUE_TYPE_BOOL_INTEL,
	e_STRING_INTEL = VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL,
	e_BEGIN_RANGE = VK_PERFORMANCE_VALUE_TYPE_BEGIN_RANGE_INTEL,
	e_END_RANGE = VK_PERFORMANCE_VALUE_TYPE_END_RANGE_INTEL,
	e_RANGE_SIZE = VK_PERFORMANCE_VALUE_TYPE_RANGE_SIZE_INTEL,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PerformanceValueTypeINTEL)
inline std::string to_string(PerformanceValueTypeINTEL value)
{
	switch(value)
	{
	case PerformanceValueTypeINTEL::e_UINT32_INTEL: return "VK_PERFORMANCE_VALUE_TYPE_UINT32_INTEL";
	case PerformanceValueTypeINTEL::e_UINT64_INTEL: return "VK_PERFORMANCE_VALUE_TYPE_UINT64_INTEL";
	case PerformanceValueTypeINTEL::e_FLOAT_INTEL: return "VK_PERFORMANCE_VALUE_TYPE_FLOAT_INTEL";
	case PerformanceValueTypeINTEL::e_BOOL_INTEL: return "VK_PERFORMANCE_VALUE_TYPE_BOOL_INTEL";
	case PerformanceValueTypeINTEL::e_STRING_INTEL: return "VK_PERFORMANCE_VALUE_TYPE_STRING_INTEL";
	default: return "invalid";
	}
}

enum class ShaderFloatControlsIndependence
{
	e_32_BIT_ONLY = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY,
	e_ALL = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL,
	e_NONE = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE,
	e_32_BIT_ONLY_KHR = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY_KHR,
	e_ALL_KHR = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL_KHR,
	e_NONE_KHR = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE_KHR,
	e_BEGIN_RANGE = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_BEGIN_RANGE,
	e_END_RANGE = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_END_RANGE,
	e_RANGE_SIZE = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_RANGE_SIZE,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(ShaderFloatControlsIndependence)
inline std::string to_string(ShaderFloatControlsIndependence value)
{
	switch(value)
	{
	case ShaderFloatControlsIndependence::e_32_BIT_ONLY: return "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY";
	case ShaderFloatControlsIndependence::e_ALL: return "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL";
	case ShaderFloatControlsIndependence::e_NONE: return "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE";
	default: return "invalid";
	}
}

enum class PipelineExecutableStatisticFormatKHR
{
	e_BOOL32_KHR = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR,
	e_INT64_KHR = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR,
	e_UINT64_KHR = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR,
	e_FLOAT64_KHR = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR,
	e_BEGIN_RANGE = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BEGIN_RANGE_KHR,
	e_END_RANGE = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_END_RANGE_KHR,
	e_RANGE_SIZE = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_RANGE_SIZE_KHR,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(PipelineExecutableStatisticFormatKHR)
inline std::string to_string(PipelineExecutableStatisticFormatKHR value)
{
	switch(value)
	{
	case PipelineExecutableStatisticFormatKHR::e_BOOL32_KHR: return "VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR";
	case PipelineExecutableStatisticFormatKHR::e_INT64_KHR: return "VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR";
	case PipelineExecutableStatisticFormatKHR::e_UINT64_KHR: return "VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR";
	case PipelineExecutableStatisticFormatKHR::e_FLOAT64_KHR: return "VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR";
	default: return "invalid";
	}
}

enum class LineRasterizationModeEXT
{
	e_DEFAULT_EXT = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT,
	e_RECTANGULAR_EXT = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT,
	e_BRESENHAM_EXT = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT,
	e_RECTANGULAR_SMOOTH_EXT = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT,
	e_BEGIN_RANGE = VK_LINE_RASTERIZATION_MODE_BEGIN_RANGE_EXT,
	e_END_RANGE = VK_LINE_RASTERIZATION_MODE_END_RANGE_EXT,
	e_RANGE_SIZE = VK_LINE_RASTERIZATION_MODE_RANGE_SIZE_EXT,
	e_MAX_ENUM = 0x7FFFFFFF
};
DEFINE_ENUM_BITWISE_OPERATORS(LineRasterizationModeEXT)
inline std::string to_string(LineRasterizationModeEXT value)
{
	switch(value)
	{
	case LineRasterizationModeEXT::e_DEFAULT_EXT: return "VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT";
	case LineRasterizationModeEXT::e_RECTANGULAR_EXT: return "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT";
	case LineRasterizationModeEXT::e_BRESENHAM_EXT: return "VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT";
	case LineRasterizationModeEXT::e_RECTANGULAR_SMOOTH_EXT: return "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT";
	default: return "invalid";
	}
}

// PVRVk Typedefs
typedef DescriptorUpdateTemplateCreateFlags DescriptorUpdateTemplateCreateFlagsKHR;
typedef SemaphoreWaitFlags SemaphoreWaitFlagsKHR;
typedef PeerMemoryFeatureFlags PeerMemoryFeatureFlagsKHR;
typedef MemoryAllocateFlags MemoryAllocateFlagsKHR;
typedef CommandPoolTrimFlags CommandPoolTrimFlagsKHR;
typedef ExternalMemoryHandleTypeFlags ExternalMemoryHandleTypeFlagsKHR;
typedef ExternalMemoryFeatureFlags ExternalMemoryFeatureFlagsKHR;
typedef ExternalSemaphoreHandleTypeFlags ExternalSemaphoreHandleTypeFlagsKHR;
typedef ExternalSemaphoreFeatureFlags ExternalSemaphoreFeatureFlagsKHR;
typedef SemaphoreImportFlags SemaphoreImportFlagsKHR;
typedef ExternalFenceHandleTypeFlags ExternalFenceHandleTypeFlagsKHR;
typedef ExternalFenceFeatureFlags ExternalFenceFeatureFlagsKHR;
typedef FenceImportFlags FenceImportFlagsKHR;
typedef DescriptorBindingFlags DescriptorBindingFlagsEXT;
typedef ResolveModeFlags ResolveModeFlagsKHR;
typedef DescriptorUpdateTemplateType DescriptorUpdateTemplateTypeKHR;
typedef PointClippingBehavior PointClippingBehaviorKHR;
typedef ResolveModeFlags ResolveModeFlagsKHR;
typedef DescriptorBindingFlags DescriptorBindingFlagsEXT;
typedef SemaphoreType SemaphoreTypeKHR;
typedef SemaphoreWaitFlags SemaphoreWaitFlagsKHR;
typedef ExternalMemoryHandleTypeFlags ExternalMemoryHandleTypeFlagsKHR;
typedef ExternalMemoryFeatureFlags ExternalMemoryFeatureFlagsKHR;
typedef ExternalSemaphoreHandleTypeFlags ExternalSemaphoreHandleTypeFlagsKHR;
typedef ExternalSemaphoreFeatureFlags ExternalSemaphoreFeatureFlagsKHR;
typedef SemaphoreImportFlags SemaphoreImportFlagsKHR;
typedef ExternalFenceHandleTypeFlags ExternalFenceHandleTypeFlagsKHR;
typedef ExternalFenceFeatureFlags ExternalFenceFeatureFlagsKHR;
typedef FenceImportFlags FenceImportFlagsKHR;
typedef PeerMemoryFeatureFlags PeerMemoryFeatureFlagsKHR;
typedef MemoryAllocateFlags MemoryAllocateFlagsKHR;
typedef TessellationDomainOrigin TessellationDomainOriginKHR;
typedef SamplerYcbcrModelConversion SamplerYcbcrModelConversionKHR;
typedef SamplerYcbcrRange SamplerYcbcrRangeKHR;
typedef ChromaLocation ChromaLocationKHR;
typedef SamplerReductionMode SamplerReductionModeEXT;
typedef ShaderFloatControlsIndependence ShaderFloatControlsIndependenceKHR;
typedef DriverId DriverIdKHR;

// PVRVk Format queries: isSrgb()
inline bool isSrgb(Format value)
{
	// This list is sorted by enum values
	const Format srgb_formats[] = {
		Format::e_R8_SRGB,
		Format::e_R8G8_SRGB,
		Format::e_R8G8B8_SRGB,
		Format::e_B8G8R8_SRGB,
		Format::e_R8G8B8A8_SRGB,
		Format::e_B8G8R8A8_SRGB,
		Format::e_A8B8G8R8_SRGB_PACK32,
		Format::e_BC1_RGB_SRGB_BLOCK,
		Format::e_BC1_RGBA_SRGB_BLOCK,
		Format::e_BC2_SRGB_BLOCK,
		Format::e_BC3_SRGB_BLOCK,
		Format::e_BC7_SRGB_BLOCK,
		Format::e_ETC2_R8G8B8_SRGB_BLOCK,
		Format::e_ETC2_R8G8B8A1_SRGB_BLOCK,
		Format::e_ETC2_R8G8B8A8_SRGB_BLOCK,
		Format::e_ASTC_4x4_SRGB_BLOCK,
		Format::e_ASTC_5x4_SRGB_BLOCK,
		Format::e_ASTC_5x5_SRGB_BLOCK,
		Format::e_ASTC_6x5_SRGB_BLOCK,
		Format::e_ASTC_6x6_SRGB_BLOCK,
		Format::e_ASTC_8x5_SRGB_BLOCK,
		Format::e_ASTC_8x6_SRGB_BLOCK,
		Format::e_ASTC_8x8_SRGB_BLOCK,
		Format::e_ASTC_10x5_SRGB_BLOCK,
		Format::e_ASTC_10x6_SRGB_BLOCK,
		Format::e_ASTC_10x8_SRGB_BLOCK,
		Format::e_ASTC_10x10_SRGB_BLOCK,
		Format::e_ASTC_12x10_SRGB_BLOCK,
		Format::e_ASTC_12x12_SRGB_BLOCK,
		Format::e_PVRTC1_2BPP_SRGB_BLOCK_IMG,
		Format::e_PVRTC1_4BPP_SRGB_BLOCK_IMG,
		Format::e_PVRTC2_2BPP_SRGB_BLOCK_IMG,
		Format::e_PVRTC2_4BPP_SRGB_BLOCK_IMG,
	};
	return std::binary_search(srgb_formats, srgb_formats + sizeof(srgb_formats)/sizeof(srgb_formats[0]), value);
}

// PVRVk Conversion : ObjectType to DebugReportObjectType
inline DebugReportObjectTypeEXT convertObjectTypeToDebugReportObjectType(ObjectType objectType) { return static_cast<DebugReportObjectTypeEXT>(objectType); }

// PVRVk Conversion : DebugReportObjectType to ObjectType 
inline ObjectType convertDebugReportObjectTypeToObjectType(DebugReportObjectTypeEXT debugReportObjectType) { return static_cast<ObjectType>(debugReportObjectType); }

// PVRVk Structures
struct Offset2D: private VkOffset2D
{
	Offset2D()
	{
		setX(int32_t());
		setY(int32_t());
	}
	Offset2D(const VkOffset2D& vkType): VkOffset2D(vkType){}
	Offset2D(int32_t x, int32_t y)
	{
		setX(x);
		setY(y);
	}
	inline int32_t getX() const { return x; }
	inline void setX(int32_t inX) { this->x = inX; }
	inline int32_t getY() const { return y; }
	inline void setY(int32_t inY) { this->y = inY; }
	inline VkOffset2D& get() { return *this; }
	inline const VkOffset2D& get() const { return *this; }
};

struct Offset3D: private VkOffset3D
{
	Offset3D()
	{
		setX(int32_t());
		setY(int32_t());
		setZ(int32_t());
	}
	Offset3D(const VkOffset3D& vkType): VkOffset3D(vkType){}
	Offset3D(int32_t x, int32_t y, int32_t z)
	{
		setX(x);
		setY(y);
		setZ(z);
	}
	inline int32_t getX() const { return x; }
	inline void setX(int32_t inX) { this->x = inX; }
	inline int32_t getY() const { return y; }
	inline void setY(int32_t inY) { this->y = inY; }
	inline int32_t getZ() const { return z; }
	inline void setZ(int32_t inZ) { this->z = inZ; }
	inline VkOffset3D& get() { return *this; }
	inline const VkOffset3D& get() const { return *this; }
};

struct Extent2D: private VkExtent2D
{
	Extent2D()
	{
		setWidth(uint32_t());
		setHeight(uint32_t());
	}
	Extent2D(const VkExtent2D& vkType): VkExtent2D(vkType){}
	Extent2D(uint32_t width, uint32_t height)
	{
		setWidth(width);
		setHeight(height);
	}
	inline uint32_t getWidth() const { return width; }
	inline void setWidth(uint32_t inWidth) { this->width = inWidth; }
	inline uint32_t getHeight() const { return height; }
	inline void setHeight(uint32_t inHeight) { this->height = inHeight; }
	inline VkExtent2D& get() { return *this; }
	inline const VkExtent2D& get() const { return *this; }
};

struct Extent3D: private VkExtent3D
{
	Extent3D()
	{
		setWidth(uint32_t());
		setHeight(uint32_t());
		setDepth(uint32_t());
	}
	Extent3D(const VkExtent3D& vkType): VkExtent3D(vkType){}
	Extent3D(uint32_t width, uint32_t height, uint32_t depth)
	{
		setWidth(width);
		setHeight(height);
		setDepth(depth);
	}
	inline uint32_t getWidth() const { return width; }
	inline void setWidth(uint32_t inWidth) { this->width = inWidth; }
	inline uint32_t getHeight() const { return height; }
	inline void setHeight(uint32_t inHeight) { this->height = inHeight; }
	inline uint32_t getDepth() const { return depth; }
	inline void setDepth(uint32_t inDepth) { this->depth = inDepth; }
	inline VkExtent3D& get() { return *this; }
	inline const VkExtent3D& get() const { return *this; }
};

struct Viewport: private VkViewport
{
	Viewport()
	{
		setX(0.f);
		setY(0.f);
		setWidth(1.f);
		setHeight(1.f);
		setMinDepth(0.f);
		setMaxDepth(1.f);
	}
	Viewport(const VkViewport& vkType): VkViewport(vkType){}
	Viewport(float x, float y, float width, float height, float minDepth = 0.f, float maxDepth = 1.f)
	{
		setX(x);
		setY(y);
		setWidth(width);
		setHeight(height);
		setMinDepth(minDepth);
		setMaxDepth(maxDepth);
	}
	inline float getX() const { return x; }
	inline void setX(float inX) { this->x = inX; }
	inline float getY() const { return y; }
	inline void setY(float inY) { this->y = inY; }
	inline float getWidth() const { return width; }
	inline void setWidth(float inWidth) { this->width = inWidth; }
	inline float getHeight() const { return height; }
	inline void setHeight(float inHeight) { this->height = inHeight; }
	inline float getMinDepth() const { return minDepth; }
	inline void setMinDepth(float inMinDepth) { this->minDepth = inMinDepth; }
	inline float getMaxDepth() const { return maxDepth; }
	inline void setMaxDepth(float inMaxDepth) { this->maxDepth = inMaxDepth; }
	inline VkViewport& get() { return *this; }
	inline const VkViewport& get() const { return *this; }
};

struct Rect2D: private VkRect2D
{
	Rect2D()
	{
		setOffset(Offset2D());
		setExtent(Extent2D());
	}
	Rect2D(const VkRect2D& vkType): VkRect2D(vkType){}
	Rect2D(const Offset2D& offset, const Extent2D& extent)
	{
		setOffset(offset);
		setExtent(extent);
	}
	Rect2D(int32_t x, int32_t y, uint32_t width, uint32_t height)
	{
		setOffset(pvrvk::Offset2D(x, y));
		setExtent(pvrvk::Extent2D(width, height));
	}
	inline const Offset2D& getOffset() const { return (Offset2D&)offset; }
	inline void setOffset(const Offset2D& inOffset) { memcpy(&this->offset, &inOffset, sizeof(this->offset)); }
	inline const Extent2D& getExtent() const { return (Extent2D&)extent; }
	inline void setExtent(const Extent2D& inExtent) { memcpy(&this->extent, &inExtent, sizeof(this->extent)); }
	inline VkRect2D& get() { return *this; }
	inline const VkRect2D& get() const { return *this; }
};

struct ClearRect: private VkClearRect
{
	ClearRect()
	{
		setRect(Rect2D());
		setBaseArrayLayer(0);
		setLayerCount(1);
	}
	ClearRect(const VkClearRect& vkType): VkClearRect(vkType){}
	ClearRect(const Rect2D& rect, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1)
	{
		setRect(rect);
		setBaseArrayLayer(baseArrayLayer);
		setLayerCount(layerCount);
	}
	inline const Rect2D& getRect() const { return (Rect2D&)rect; }
	inline void setRect(const Rect2D& inRect) { memcpy(&this->rect, &inRect, sizeof(this->rect)); }
	inline uint32_t getBaseArrayLayer() const { return baseArrayLayer; }
	inline void setBaseArrayLayer(uint32_t inBaseArrayLayer) { this->baseArrayLayer = inBaseArrayLayer; }
	inline uint32_t getLayerCount() const { return layerCount; }
	inline void setLayerCount(uint32_t inLayerCount) { this->layerCount = inLayerCount; }
	inline VkClearRect& get() { return *this; }
	inline const VkClearRect& get() const { return *this; }
};

struct ComponentMapping: private VkComponentMapping
{
	ComponentMapping()
	{
		setR(ComponentSwizzle());
		setG(ComponentSwizzle());
		setB(ComponentSwizzle());
		setA(ComponentSwizzle());
	}
	ComponentMapping(const VkComponentMapping& vkType): VkComponentMapping(vkType){}
	ComponentMapping(ComponentSwizzle r, ComponentSwizzle g, ComponentSwizzle b, ComponentSwizzle a)
	{
		setR(r);
		setG(g);
		setB(b);
		setA(a);
	}
	inline ComponentSwizzle getR() const { return (ComponentSwizzle)r; }
	inline void setR(ComponentSwizzle inR) { this->r = (VkComponentSwizzle)inR; }
	inline ComponentSwizzle getG() const { return (ComponentSwizzle)g; }
	inline void setG(ComponentSwizzle inG) { this->g = (VkComponentSwizzle)inG; }
	inline ComponentSwizzle getB() const { return (ComponentSwizzle)b; }
	inline void setB(ComponentSwizzle inB) { this->b = (VkComponentSwizzle)inB; }
	inline ComponentSwizzle getA() const { return (ComponentSwizzle)a; }
	inline void setA(ComponentSwizzle inA) { this->a = (VkComponentSwizzle)inA; }
	inline VkComponentMapping& get() { return *this; }
	inline const VkComponentMapping& get() const { return *this; }
};

// PhysicalDeviceLimits is a structure used only as a return type so only getters are defined
struct PhysicalDeviceLimits: private VkPhysicalDeviceLimits
{
	PhysicalDeviceLimits(){}
	PhysicalDeviceLimits(const VkPhysicalDeviceLimits& vkType): VkPhysicalDeviceLimits(vkType){}
	inline uint32_t getMaxImageDimension1D() const { return maxImageDimension1D; }
	inline uint32_t getMaxImageDimension2D() const { return maxImageDimension2D; }
	inline uint32_t getMaxImageDimension3D() const { return maxImageDimension3D; }
	inline uint32_t getMaxImageDimensionCube() const { return maxImageDimensionCube; }
	inline uint32_t getMaxImageArrayLayers() const { return maxImageArrayLayers; }
	inline uint32_t getMaxTexelBufferElements() const { return maxTexelBufferElements; }
	inline uint32_t getMaxUniformBufferRange() const { return maxUniformBufferRange; }
	inline uint32_t getMaxStorageBufferRange() const { return maxStorageBufferRange; }
	inline uint32_t getMaxPushConstantsSize() const { return maxPushConstantsSize; }
	inline uint32_t getMaxMemoryAllocationCount() const { return maxMemoryAllocationCount; }
	inline uint32_t getMaxSamplerAllocationCount() const { return maxSamplerAllocationCount; }
	inline VkDeviceSize getBufferImageGranularity() const { return bufferImageGranularity; }
	inline VkDeviceSize getSparseAddressSpaceSize() const { return sparseAddressSpaceSize; }
	inline uint32_t getMaxBoundDescriptorSets() const { return maxBoundDescriptorSets; }
	inline uint32_t getMaxPerStageDescriptorSamplers() const { return maxPerStageDescriptorSamplers; }
	inline uint32_t getMaxPerStageDescriptorUniformBuffers() const { return maxPerStageDescriptorUniformBuffers; }
	inline uint32_t getMaxPerStageDescriptorStorageBuffers() const { return maxPerStageDescriptorStorageBuffers; }
	inline uint32_t getMaxPerStageDescriptorSampledImages() const { return maxPerStageDescriptorSampledImages; }
	inline uint32_t getMaxPerStageDescriptorStorageImages() const { return maxPerStageDescriptorStorageImages; }
	inline uint32_t getMaxPerStageDescriptorInputAttachments() const { return maxPerStageDescriptorInputAttachments; }
	inline uint32_t getMaxPerStageResources() const { return maxPerStageResources; }
	inline uint32_t getMaxDescriptorSetSamplers() const { return maxDescriptorSetSamplers; }
	inline uint32_t getMaxDescriptorSetUniformBuffers() const { return maxDescriptorSetUniformBuffers; }
	inline uint32_t getMaxDescriptorSetUniformBuffersDynamic() const { return maxDescriptorSetUniformBuffersDynamic; }
	inline uint32_t getMaxDescriptorSetStorageBuffers() const { return maxDescriptorSetStorageBuffers; }
	inline uint32_t getMaxDescriptorSetStorageBuffersDynamic() const { return maxDescriptorSetStorageBuffersDynamic; }
	inline uint32_t getMaxDescriptorSetSampledImages() const { return maxDescriptorSetSampledImages; }
	inline uint32_t getMaxDescriptorSetStorageImages() const { return maxDescriptorSetStorageImages; }
	inline uint32_t getMaxDescriptorSetInputAttachments() const { return maxDescriptorSetInputAttachments; }
	inline uint32_t getMaxVertexInputAttributes() const { return maxVertexInputAttributes; }
	inline uint32_t getMaxVertexInputBindings() const { return maxVertexInputBindings; }
	inline uint32_t getMaxVertexInputAttributeOffset() const { return maxVertexInputAttributeOffset; }
	inline uint32_t getMaxVertexInputBindingStride() const { return maxVertexInputBindingStride; }
	inline uint32_t getMaxVertexOutputComponents() const { return maxVertexOutputComponents; }
	inline uint32_t getMaxTessellationGenerationLevel() const { return maxTessellationGenerationLevel; }
	inline uint32_t getMaxTessellationPatchSize() const { return maxTessellationPatchSize; }
	inline uint32_t getMaxTessellationControlPerVertexInputComponents() const { return maxTessellationControlPerVertexInputComponents; }
	inline uint32_t getMaxTessellationControlPerVertexOutputComponents() const { return maxTessellationControlPerVertexOutputComponents; }
	inline uint32_t getMaxTessellationControlPerPatchOutputComponents() const { return maxTessellationControlPerPatchOutputComponents; }
	inline uint32_t getMaxTessellationControlTotalOutputComponents() const { return maxTessellationControlTotalOutputComponents; }
	inline uint32_t getMaxTessellationEvaluationInputComponents() const { return maxTessellationEvaluationInputComponents; }
	inline uint32_t getMaxTessellationEvaluationOutputComponents() const { return maxTessellationEvaluationOutputComponents; }
	inline uint32_t getMaxGeometryShaderInvocations() const { return maxGeometryShaderInvocations; }
	inline uint32_t getMaxGeometryInputComponents() const { return maxGeometryInputComponents; }
	inline uint32_t getMaxGeometryOutputComponents() const { return maxGeometryOutputComponents; }
	inline uint32_t getMaxGeometryOutputVertices() const { return maxGeometryOutputVertices; }
	inline uint32_t getMaxGeometryTotalOutputComponents() const { return maxGeometryTotalOutputComponents; }
	inline uint32_t getMaxFragmentInputComponents() const { return maxFragmentInputComponents; }
	inline uint32_t getMaxFragmentOutputAttachments() const { return maxFragmentOutputAttachments; }
	inline uint32_t getMaxFragmentDualSrcAttachments() const { return maxFragmentDualSrcAttachments; }
	inline uint32_t getMaxFragmentCombinedOutputResources() const { return maxFragmentCombinedOutputResources; }
	inline uint32_t getMaxComputeSharedMemorySize() const { return maxComputeSharedMemorySize; }
	inline const uint32_t* getMaxComputeWorkGroupCount() const { return maxComputeWorkGroupCount; }
	inline uint32_t getMaxComputeWorkGroupInvocations() const { return maxComputeWorkGroupInvocations; }
	inline const uint32_t* getMaxComputeWorkGroupSize() const { return maxComputeWorkGroupSize; }
	inline uint32_t getSubPixelPrecisionBits() const { return subPixelPrecisionBits; }
	inline uint32_t getSubTexelPrecisionBits() const { return subTexelPrecisionBits; }
	inline uint32_t getMipmapPrecisionBits() const { return mipmapPrecisionBits; }
	inline uint32_t getMaxDrawIndexedIndexValue() const { return maxDrawIndexedIndexValue; }
	inline uint32_t getMaxDrawIndirectCount() const { return maxDrawIndirectCount; }
	inline float getMaxSamplerLodBias() const { return maxSamplerLodBias; }
	inline float getMaxSamplerAnisotropy() const { return maxSamplerAnisotropy; }
	inline uint32_t getMaxViewports() const { return maxViewports; }
	inline const uint32_t* getMaxViewportDimensions() const { return maxViewportDimensions; }
	inline const float* getViewportBoundsRange() const { return viewportBoundsRange; }
	inline uint32_t getViewportSubPixelBits() const { return viewportSubPixelBits; }
	inline size_t getMinMemoryMapAlignment() const { return minMemoryMapAlignment; }
	inline VkDeviceSize getMinTexelBufferOffsetAlignment() const { return minTexelBufferOffsetAlignment; }
	inline VkDeviceSize getMinUniformBufferOffsetAlignment() const { return minUniformBufferOffsetAlignment; }
	inline VkDeviceSize getMinStorageBufferOffsetAlignment() const { return minStorageBufferOffsetAlignment; }
	inline int32_t getMinTexelOffset() const { return minTexelOffset; }
	inline uint32_t getMaxTexelOffset() const { return maxTexelOffset; }
	inline int32_t getMinTexelGatherOffset() const { return minTexelGatherOffset; }
	inline uint32_t getMaxTexelGatherOffset() const { return maxTexelGatherOffset; }
	inline float getMinInterpolationOffset() const { return minInterpolationOffset; }
	inline float getMaxInterpolationOffset() const { return maxInterpolationOffset; }
	inline uint32_t getSubPixelInterpolationOffsetBits() const { return subPixelInterpolationOffsetBits; }
	inline uint32_t getMaxFramebufferWidth() const { return maxFramebufferWidth; }
	inline uint32_t getMaxFramebufferHeight() const { return maxFramebufferHeight; }
	inline uint32_t getMaxFramebufferLayers() const { return maxFramebufferLayers; }
	inline SampleCountFlags getFramebufferColorSampleCounts() const { return (SampleCountFlags)framebufferColorSampleCounts; }
	inline SampleCountFlags getFramebufferDepthSampleCounts() const { return (SampleCountFlags)framebufferDepthSampleCounts; }
	inline SampleCountFlags getFramebufferStencilSampleCounts() const { return (SampleCountFlags)framebufferStencilSampleCounts; }
	inline SampleCountFlags getFramebufferNoAttachmentsSampleCounts() const { return (SampleCountFlags)framebufferNoAttachmentsSampleCounts; }
	inline uint32_t getMaxColorAttachments() const { return maxColorAttachments; }
	inline SampleCountFlags getSampledImageColorSampleCounts() const { return (SampleCountFlags)sampledImageColorSampleCounts; }
	inline SampleCountFlags getSampledImageIntegerSampleCounts() const { return (SampleCountFlags)sampledImageIntegerSampleCounts; }
	inline SampleCountFlags getSampledImageDepthSampleCounts() const { return (SampleCountFlags)sampledImageDepthSampleCounts; }
	inline SampleCountFlags getSampledImageStencilSampleCounts() const { return (SampleCountFlags)sampledImageStencilSampleCounts; }
	inline SampleCountFlags getStorageImageSampleCounts() const { return (SampleCountFlags)storageImageSampleCounts; }
	inline uint32_t getMaxSampleMaskWords() const { return maxSampleMaskWords; }
	inline VkBool32 getTimestampComputeAndGraphics() const { return timestampComputeAndGraphics; }
	inline float getTimestampPeriod() const { return timestampPeriod; }
	inline uint32_t getMaxClipDistances() const { return maxClipDistances; }
	inline uint32_t getMaxCullDistances() const { return maxCullDistances; }
	inline uint32_t getMaxCombinedClipAndCullDistances() const { return maxCombinedClipAndCullDistances; }
	inline uint32_t getDiscreteQueuePriorities() const { return discreteQueuePriorities; }
	inline const float* getPointSizeRange() const { return pointSizeRange; }
	inline const float* getLineWidthRange() const { return lineWidthRange; }
	inline float getPointSizeGranularity() const { return pointSizeGranularity; }
	inline float getLineWidthGranularity() const { return lineWidthGranularity; }
	inline VkBool32 getStrictLines() const { return strictLines; }
	inline VkBool32 getStandardSampleLocations() const { return standardSampleLocations; }
	inline VkDeviceSize getOptimalBufferCopyOffsetAlignment() const { return optimalBufferCopyOffsetAlignment; }
	inline VkDeviceSize getOptimalBufferCopyRowPitchAlignment() const { return optimalBufferCopyRowPitchAlignment; }
	inline VkDeviceSize getNonCoherentAtomSize() const { return nonCoherentAtomSize; }
	inline VkPhysicalDeviceLimits& get() { return *this; }
	inline const VkPhysicalDeviceLimits& get() const { return *this; }
};

// PhysicalDeviceSparseProperties is a structure used only as a return type so only getters are defined
struct PhysicalDeviceSparseProperties: private VkPhysicalDeviceSparseProperties
{
	PhysicalDeviceSparseProperties(){}
	PhysicalDeviceSparseProperties(const VkPhysicalDeviceSparseProperties& vkType): VkPhysicalDeviceSparseProperties(vkType){}
	inline VkBool32 getResidencyStandard2DBlockShape() const { return residencyStandard2DBlockShape; }
	inline VkBool32 getResidencyStandard2DMultisampleBlockShape() const { return residencyStandard2DMultisampleBlockShape; }
	inline VkBool32 getResidencyStandard3DBlockShape() const { return residencyStandard3DBlockShape; }
	inline VkBool32 getResidencyAlignedMipSize() const { return residencyAlignedMipSize; }
	inline VkBool32 getResidencyNonResidentStrict() const { return residencyNonResidentStrict; }
	inline VkPhysicalDeviceSparseProperties& get() { return *this; }
	inline const VkPhysicalDeviceSparseProperties& get() const { return *this; }
};

// PhysicalDeviceProperties is a structure used only as a return type so only getters are defined
struct PhysicalDeviceProperties: private VkPhysicalDeviceProperties
{
	PhysicalDeviceProperties(){}
	PhysicalDeviceProperties(const VkPhysicalDeviceProperties& vkType): VkPhysicalDeviceProperties(vkType){}
	inline uint32_t getApiVersion() const { return apiVersion; }
	inline uint32_t getDriverVersion() const { return driverVersion; }
	inline uint32_t getVendorID() const { return vendorID; }
	inline uint32_t getDeviceID() const { return deviceID; }
	inline PhysicalDeviceType getDeviceType() const { return (PhysicalDeviceType)deviceType; }
	inline const char* getDeviceName() const { return deviceName; }
	inline const uint8_t* getPipelineCacheUUID() const { return pipelineCacheUUID; }
	inline const PhysicalDeviceLimits& getLimits() const { return (PhysicalDeviceLimits&)limits; }
	inline const PhysicalDeviceSparseProperties& getSparseProperties() const { return (PhysicalDeviceSparseProperties&)sparseProperties; }
	inline VkPhysicalDeviceProperties& get() { return *this; }
	inline const VkPhysicalDeviceProperties& get() const { return *this; }
};

// ExtensionProperties is a structure used only as a return type so only getters are defined
struct ExtensionProperties: private VkExtensionProperties
{
	ExtensionProperties(){}
	ExtensionProperties(const VkExtensionProperties& vkType): VkExtensionProperties(vkType){}
	inline const char* getExtensionName() const { return extensionName; }
	inline uint32_t getSpecVersion() const { return specVersion; }
	inline VkExtensionProperties& get() { return *this; }
	inline const VkExtensionProperties& get() const { return *this; }
};

// LayerProperties is a structure used only as a return type so only getters are defined
struct LayerProperties: private VkLayerProperties
{
	LayerProperties(){}
	LayerProperties(const VkLayerProperties& vkType): VkLayerProperties(vkType){}
	inline const char* getLayerName() const { return layerName; }
	inline uint32_t getSpecVersion() const { return specVersion; }
	inline uint32_t getImplementationVersion() const { return implementationVersion; }
	inline const char* getDescription() const { return description; }
	inline VkLayerProperties& get() { return *this; }
	inline const VkLayerProperties& get() const { return *this; }
};

struct AllocationCallbacks: private VkAllocationCallbacks
{
	AllocationCallbacks()
	{
		setPUserData(nullptr);
		setPfnAllocation(PFN_vkAllocationFunction());
		setPfnReallocation(PFN_vkReallocationFunction());
		setPfnFree(PFN_vkFreeFunction());
		setPfnInternalAllocation(PFN_vkInternalAllocationNotification());
		setPfnInternalFree(PFN_vkInternalFreeNotification());
	}
	AllocationCallbacks(const VkAllocationCallbacks& vkType): VkAllocationCallbacks(vkType){}
	AllocationCallbacks(void* pUserData, PFN_vkAllocationFunction pfnAllocation, PFN_vkReallocationFunction pfnReallocation, PFN_vkFreeFunction pfnFree, PFN_vkInternalAllocationNotification pfnInternalAllocation, PFN_vkInternalFreeNotification pfnInternalFree)
	{
		setPUserData(pUserData);
		setPfnAllocation(pfnAllocation);
		setPfnReallocation(pfnReallocation);
		setPfnFree(pfnFree);
		setPfnInternalAllocation(pfnInternalAllocation);
		setPfnInternalFree(pfnInternalFree);
	}
	inline const void* getPUserData() const { return pUserData; }
	inline void setPUserData(void* inPUserData) { this->pUserData = inPUserData; }
	inline PFN_vkAllocationFunction getPfnAllocation() const { return pfnAllocation; }
	inline void setPfnAllocation(PFN_vkAllocationFunction inPfnAllocation) { this->pfnAllocation = inPfnAllocation; }
	inline PFN_vkReallocationFunction getPfnReallocation() const { return pfnReallocation; }
	inline void setPfnReallocation(PFN_vkReallocationFunction inPfnReallocation) { this->pfnReallocation = inPfnReallocation; }
	inline PFN_vkFreeFunction getPfnFree() const { return pfnFree; }
	inline void setPfnFree(PFN_vkFreeFunction inPfnFree) { this->pfnFree = inPfnFree; }
	inline PFN_vkInternalAllocationNotification getPfnInternalAllocation() const { return pfnInternalAllocation; }
	inline void setPfnInternalAllocation(PFN_vkInternalAllocationNotification inPfnInternalAllocation) { this->pfnInternalAllocation = inPfnInternalAllocation; }
	inline PFN_vkInternalFreeNotification getPfnInternalFree() const { return pfnInternalFree; }
	inline void setPfnInternalFree(PFN_vkInternalFreeNotification inPfnInternalFree) { this->pfnInternalFree = inPfnInternalFree; }
	inline VkAllocationCallbacks& get() { return *this; }
	inline const VkAllocationCallbacks& get() const { return *this; }
};

struct PhysicalDeviceFeatures: private VkPhysicalDeviceFeatures
{
	PhysicalDeviceFeatures()
	{
		setRobustBufferAccess(VkBool32());
		setFullDrawIndexUint32(VkBool32());
		setImageCubeArray(VkBool32());
		setIndependentBlend(VkBool32());
		setGeometryShader(VkBool32());
		setTessellationShader(VkBool32());
		setSampleRateShading(VkBool32());
		setDualSrcBlend(VkBool32());
		setLogicOp(VkBool32());
		setMultiDrawIndirect(VkBool32());
		setDrawIndirectFirstInstance(VkBool32());
		setDepthClamp(VkBool32());
		setDepthBiasClamp(VkBool32());
		setFillModeNonSolid(VkBool32());
		setDepthBounds(VkBool32());
		setWideLines(VkBool32());
		setLargePoints(VkBool32());
		setAlphaToOne(VkBool32());
		setMultiViewport(VkBool32());
		setSamplerAnisotropy(VkBool32());
		setTextureCompressionETC2(VkBool32());
		setTextureCompressionASTC_LDR(VkBool32());
		setTextureCompressionBC(VkBool32());
		setOcclusionQueryPrecise(VkBool32());
		setPipelineStatisticsQuery(VkBool32());
		setVertexPipelineStoresAndAtomics(VkBool32());
		setFragmentStoresAndAtomics(VkBool32());
		setShaderTessellationAndGeometryPointSize(VkBool32());
		setShaderImageGatherExtended(VkBool32());
		setShaderStorageImageExtendedFormats(VkBool32());
		setShaderStorageImageMultisample(VkBool32());
		setShaderStorageImageReadWithoutFormat(VkBool32());
		setShaderStorageImageWriteWithoutFormat(VkBool32());
		setShaderUniformBufferArrayDynamicIndexing(VkBool32());
		setShaderSampledImageArrayDynamicIndexing(VkBool32());
		setShaderStorageBufferArrayDynamicIndexing(VkBool32());
		setShaderStorageImageArrayDynamicIndexing(VkBool32());
		setShaderClipDistance(VkBool32());
		setShaderCullDistance(VkBool32());
		setShaderFloat64(VkBool32());
		setShaderInt64(VkBool32());
		setShaderInt16(VkBool32());
		setShaderResourceResidency(VkBool32());
		setShaderResourceMinLod(VkBool32());
		setSparseBinding(VkBool32());
		setSparseResidencyBuffer(VkBool32());
		setSparseResidencyImage2D(VkBool32());
		setSparseResidencyImage3D(VkBool32());
		setSparseResidency2Samples(VkBool32());
		setSparseResidency4Samples(VkBool32());
		setSparseResidency8Samples(VkBool32());
		setSparseResidency16Samples(VkBool32());
		setSparseResidencyAliased(VkBool32());
		setVariableMultisampleRate(VkBool32());
		setInheritedQueries(VkBool32());
	}
	PhysicalDeviceFeatures(const VkPhysicalDeviceFeatures& vkType): VkPhysicalDeviceFeatures(vkType){}
	PhysicalDeviceFeatures(VkBool32 robustBufferAccess, VkBool32 fullDrawIndexUint32, VkBool32 imageCubeArray, VkBool32 independentBlend, VkBool32 geometryShader, VkBool32 tessellationShader, VkBool32 sampleRateShading, VkBool32 dualSrcBlend, VkBool32 logicOp, VkBool32 multiDrawIndirect, VkBool32 drawIndirectFirstInstance, VkBool32 depthClamp, VkBool32 depthBiasClamp, VkBool32 fillModeNonSolid, VkBool32 depthBounds, VkBool32 wideLines, VkBool32 largePoints, VkBool32 alphaToOne, VkBool32 multiViewport, VkBool32 samplerAnisotropy, VkBool32 textureCompressionETC2, VkBool32 textureCompressionASTC_LDR, VkBool32 textureCompressionBC, VkBool32 occlusionQueryPrecise, VkBool32 pipelineStatisticsQuery, VkBool32 vertexPipelineStoresAndAtomics, VkBool32 fragmentStoresAndAtomics, VkBool32 shaderTessellationAndGeometryPointSize, VkBool32 shaderImageGatherExtended, VkBool32 shaderStorageImageExtendedFormats, VkBool32 shaderStorageImageMultisample, VkBool32 shaderStorageImageReadWithoutFormat, VkBool32 shaderStorageImageWriteWithoutFormat, VkBool32 shaderUniformBufferArrayDynamicIndexing, VkBool32 shaderSampledImageArrayDynamicIndexing, VkBool32 shaderStorageBufferArrayDynamicIndexing, VkBool32 shaderStorageImageArrayDynamicIndexing, VkBool32 shaderClipDistance, VkBool32 shaderCullDistance, VkBool32 shaderFloat64, VkBool32 shaderInt64, VkBool32 shaderInt16, VkBool32 shaderResourceResidency, VkBool32 shaderResourceMinLod, VkBool32 sparseBinding, VkBool32 sparseResidencyBuffer, VkBool32 sparseResidencyImage2D, VkBool32 sparseResidencyImage3D, VkBool32 sparseResidency2Samples, VkBool32 sparseResidency4Samples, VkBool32 sparseResidency8Samples, VkBool32 sparseResidency16Samples, VkBool32 sparseResidencyAliased, VkBool32 variableMultisampleRate, VkBool32 inheritedQueries)
	{
		setRobustBufferAccess(robustBufferAccess);
		setFullDrawIndexUint32(fullDrawIndexUint32);
		setImageCubeArray(imageCubeArray);
		setIndependentBlend(independentBlend);
		setGeometryShader(geometryShader);
		setTessellationShader(tessellationShader);
		setSampleRateShading(sampleRateShading);
		setDualSrcBlend(dualSrcBlend);
		setLogicOp(logicOp);
		setMultiDrawIndirect(multiDrawIndirect);
		setDrawIndirectFirstInstance(drawIndirectFirstInstance);
		setDepthClamp(depthClamp);
		setDepthBiasClamp(depthBiasClamp);
		setFillModeNonSolid(fillModeNonSolid);
		setDepthBounds(depthBounds);
		setWideLines(wideLines);
		setLargePoints(largePoints);
		setAlphaToOne(alphaToOne);
		setMultiViewport(multiViewport);
		setSamplerAnisotropy(samplerAnisotropy);
		setTextureCompressionETC2(textureCompressionETC2);
		setTextureCompressionASTC_LDR(textureCompressionASTC_LDR);
		setTextureCompressionBC(textureCompressionBC);
		setOcclusionQueryPrecise(occlusionQueryPrecise);
		setPipelineStatisticsQuery(pipelineStatisticsQuery);
		setVertexPipelineStoresAndAtomics(vertexPipelineStoresAndAtomics);
		setFragmentStoresAndAtomics(fragmentStoresAndAtomics);
		setShaderTessellationAndGeometryPointSize(shaderTessellationAndGeometryPointSize);
		setShaderImageGatherExtended(shaderImageGatherExtended);
		setShaderStorageImageExtendedFormats(shaderStorageImageExtendedFormats);
		setShaderStorageImageMultisample(shaderStorageImageMultisample);
		setShaderStorageImageReadWithoutFormat(shaderStorageImageReadWithoutFormat);
		setShaderStorageImageWriteWithoutFormat(shaderStorageImageWriteWithoutFormat);
		setShaderUniformBufferArrayDynamicIndexing(shaderUniformBufferArrayDynamicIndexing);
		setShaderSampledImageArrayDynamicIndexing(shaderSampledImageArrayDynamicIndexing);
		setShaderStorageBufferArrayDynamicIndexing(shaderStorageBufferArrayDynamicIndexing);
		setShaderStorageImageArrayDynamicIndexing(shaderStorageImageArrayDynamicIndexing);
		setShaderClipDistance(shaderClipDistance);
		setShaderCullDistance(shaderCullDistance);
		setShaderFloat64(shaderFloat64);
		setShaderInt64(shaderInt64);
		setShaderInt16(shaderInt16);
		setShaderResourceResidency(shaderResourceResidency);
		setShaderResourceMinLod(shaderResourceMinLod);
		setSparseBinding(sparseBinding);
		setSparseResidencyBuffer(sparseResidencyBuffer);
		setSparseResidencyImage2D(sparseResidencyImage2D);
		setSparseResidencyImage3D(sparseResidencyImage3D);
		setSparseResidency2Samples(sparseResidency2Samples);
		setSparseResidency4Samples(sparseResidency4Samples);
		setSparseResidency8Samples(sparseResidency8Samples);
		setSparseResidency16Samples(sparseResidency16Samples);
		setSparseResidencyAliased(sparseResidencyAliased);
		setVariableMultisampleRate(variableMultisampleRate);
		setInheritedQueries(inheritedQueries);
	}
	inline VkBool32 getRobustBufferAccess() const { return robustBufferAccess; }
	inline void setRobustBufferAccess(VkBool32 inRobustBufferAccess) { this->robustBufferAccess = inRobustBufferAccess; }
	inline VkBool32 getFullDrawIndexUint32() const { return fullDrawIndexUint32; }
	inline void setFullDrawIndexUint32(VkBool32 inFullDrawIndexUint32) { this->fullDrawIndexUint32 = inFullDrawIndexUint32; }
	inline VkBool32 getImageCubeArray() const { return imageCubeArray; }
	inline void setImageCubeArray(VkBool32 inImageCubeArray) { this->imageCubeArray = inImageCubeArray; }
	inline VkBool32 getIndependentBlend() const { return independentBlend; }
	inline void setIndependentBlend(VkBool32 inIndependentBlend) { this->independentBlend = inIndependentBlend; }
	inline VkBool32 getGeometryShader() const { return geometryShader; }
	inline void setGeometryShader(VkBool32 inGeometryShader) { this->geometryShader = inGeometryShader; }
	inline VkBool32 getTessellationShader() const { return tessellationShader; }
	inline void setTessellationShader(VkBool32 inTessellationShader) { this->tessellationShader = inTessellationShader; }
	inline VkBool32 getSampleRateShading() const { return sampleRateShading; }
	inline void setSampleRateShading(VkBool32 inSampleRateShading) { this->sampleRateShading = inSampleRateShading; }
	inline VkBool32 getDualSrcBlend() const { return dualSrcBlend; }
	inline void setDualSrcBlend(VkBool32 inDualSrcBlend) { this->dualSrcBlend = inDualSrcBlend; }
	inline VkBool32 getLogicOp() const { return logicOp; }
	inline void setLogicOp(VkBool32 inLogicOp) { this->logicOp = inLogicOp; }
	inline VkBool32 getMultiDrawIndirect() const { return multiDrawIndirect; }
	inline void setMultiDrawIndirect(VkBool32 inMultiDrawIndirect) { this->multiDrawIndirect = inMultiDrawIndirect; }
	inline VkBool32 getDrawIndirectFirstInstance() const { return drawIndirectFirstInstance; }
	inline void setDrawIndirectFirstInstance(VkBool32 inDrawIndirectFirstInstance) { this->drawIndirectFirstInstance = inDrawIndirectFirstInstance; }
	inline VkBool32 getDepthClamp() const { return depthClamp; }
	inline void setDepthClamp(VkBool32 inDepthClamp) { this->depthClamp = inDepthClamp; }
	inline VkBool32 getDepthBiasClamp() const { return depthBiasClamp; }
	inline void setDepthBiasClamp(VkBool32 inDepthBiasClamp) { this->depthBiasClamp = inDepthBiasClamp; }
	inline VkBool32 getFillModeNonSolid() const { return fillModeNonSolid; }
	inline void setFillModeNonSolid(VkBool32 inFillModeNonSolid) { this->fillModeNonSolid = inFillModeNonSolid; }
	inline VkBool32 getDepthBounds() const { return depthBounds; }
	inline void setDepthBounds(VkBool32 inDepthBounds) { this->depthBounds = inDepthBounds; }
	inline VkBool32 getWideLines() const { return wideLines; }
	inline void setWideLines(VkBool32 inWideLines) { this->wideLines = inWideLines; }
	inline VkBool32 getLargePoints() const { return largePoints; }
	inline void setLargePoints(VkBool32 inLargePoints) { this->largePoints = inLargePoints; }
	inline VkBool32 getAlphaToOne() const { return alphaToOne; }
	inline void setAlphaToOne(VkBool32 inAlphaToOne) { this->alphaToOne = inAlphaToOne; }
	inline VkBool32 getMultiViewport() const { return multiViewport; }
	inline void setMultiViewport(VkBool32 inMultiViewport) { this->multiViewport = inMultiViewport; }
	inline VkBool32 getSamplerAnisotropy() const { return samplerAnisotropy; }
	inline void setSamplerAnisotropy(VkBool32 inSamplerAnisotropy) { this->samplerAnisotropy = inSamplerAnisotropy; }
	inline VkBool32 getTextureCompressionETC2() const { return textureCompressionETC2; }
	inline void setTextureCompressionETC2(VkBool32 inTextureCompressionETC2) { this->textureCompressionETC2 = inTextureCompressionETC2; }
	inline VkBool32 getTextureCompressionASTC_LDR() const { return textureCompressionASTC_LDR; }
	inline void setTextureCompressionASTC_LDR(VkBool32 inTextureCompressionASTC_LDR) { this->textureCompressionASTC_LDR = inTextureCompressionASTC_LDR; }
	inline VkBool32 getTextureCompressionBC() const { return textureCompressionBC; }
	inline void setTextureCompressionBC(VkBool32 inTextureCompressionBC) { this->textureCompressionBC = inTextureCompressionBC; }
	inline VkBool32 getOcclusionQueryPrecise() const { return occlusionQueryPrecise; }
	inline void setOcclusionQueryPrecise(VkBool32 inOcclusionQueryPrecise) { this->occlusionQueryPrecise = inOcclusionQueryPrecise; }
	inline VkBool32 getPipelineStatisticsQuery() const { return pipelineStatisticsQuery; }
	inline void setPipelineStatisticsQuery(VkBool32 inPipelineStatisticsQuery) { this->pipelineStatisticsQuery = inPipelineStatisticsQuery; }
	inline VkBool32 getVertexPipelineStoresAndAtomics() const { return vertexPipelineStoresAndAtomics; }
	inline void setVertexPipelineStoresAndAtomics(VkBool32 inVertexPipelineStoresAndAtomics) { this->vertexPipelineStoresAndAtomics = inVertexPipelineStoresAndAtomics; }
	inline VkBool32 getFragmentStoresAndAtomics() const { return fragmentStoresAndAtomics; }
	inline void setFragmentStoresAndAtomics(VkBool32 inFragmentStoresAndAtomics) { this->fragmentStoresAndAtomics = inFragmentStoresAndAtomics; }
	inline VkBool32 getShaderTessellationAndGeometryPointSize() const { return shaderTessellationAndGeometryPointSize; }
	inline void setShaderTessellationAndGeometryPointSize(VkBool32 inShaderTessellationAndGeometryPointSize) { this->shaderTessellationAndGeometryPointSize = inShaderTessellationAndGeometryPointSize; }
	inline VkBool32 getShaderImageGatherExtended() const { return shaderImageGatherExtended; }
	inline void setShaderImageGatherExtended(VkBool32 inShaderImageGatherExtended) { this->shaderImageGatherExtended = inShaderImageGatherExtended; }
	inline VkBool32 getShaderStorageImageExtendedFormats() const { return shaderStorageImageExtendedFormats; }
	inline void setShaderStorageImageExtendedFormats(VkBool32 inShaderStorageImageExtendedFormats) { this->shaderStorageImageExtendedFormats = inShaderStorageImageExtendedFormats; }
	inline VkBool32 getShaderStorageImageMultisample() const { return shaderStorageImageMultisample; }
	inline void setShaderStorageImageMultisample(VkBool32 inShaderStorageImageMultisample) { this->shaderStorageImageMultisample = inShaderStorageImageMultisample; }
	inline VkBool32 getShaderStorageImageReadWithoutFormat() const { return shaderStorageImageReadWithoutFormat; }
	inline void setShaderStorageImageReadWithoutFormat(VkBool32 inShaderStorageImageReadWithoutFormat) { this->shaderStorageImageReadWithoutFormat = inShaderStorageImageReadWithoutFormat; }
	inline VkBool32 getShaderStorageImageWriteWithoutFormat() const { return shaderStorageImageWriteWithoutFormat; }
	inline void setShaderStorageImageWriteWithoutFormat(VkBool32 inShaderStorageImageWriteWithoutFormat) { this->shaderStorageImageWriteWithoutFormat = inShaderStorageImageWriteWithoutFormat; }
	inline VkBool32 getShaderUniformBufferArrayDynamicIndexing() const { return shaderUniformBufferArrayDynamicIndexing; }
	inline void setShaderUniformBufferArrayDynamicIndexing(VkBool32 inShaderUniformBufferArrayDynamicIndexing) { this->shaderUniformBufferArrayDynamicIndexing = inShaderUniformBufferArrayDynamicIndexing; }
	inline VkBool32 getShaderSampledImageArrayDynamicIndexing() const { return shaderSampledImageArrayDynamicIndexing; }
	inline void setShaderSampledImageArrayDynamicIndexing(VkBool32 inShaderSampledImageArrayDynamicIndexing) { this->shaderSampledImageArrayDynamicIndexing = inShaderSampledImageArrayDynamicIndexing; }
	inline VkBool32 getShaderStorageBufferArrayDynamicIndexing() const { return shaderStorageBufferArrayDynamicIndexing; }
	inline void setShaderStorageBufferArrayDynamicIndexing(VkBool32 inShaderStorageBufferArrayDynamicIndexing) { this->shaderStorageBufferArrayDynamicIndexing = inShaderStorageBufferArrayDynamicIndexing; }
	inline VkBool32 getShaderStorageImageArrayDynamicIndexing() const { return shaderStorageImageArrayDynamicIndexing; }
	inline void setShaderStorageImageArrayDynamicIndexing(VkBool32 inShaderStorageImageArrayDynamicIndexing) { this->shaderStorageImageArrayDynamicIndexing = inShaderStorageImageArrayDynamicIndexing; }
	inline VkBool32 getShaderClipDistance() const { return shaderClipDistance; }
	inline void setShaderClipDistance(VkBool32 inShaderClipDistance) { this->shaderClipDistance = inShaderClipDistance; }
	inline VkBool32 getShaderCullDistance() const { return shaderCullDistance; }
	inline void setShaderCullDistance(VkBool32 inShaderCullDistance) { this->shaderCullDistance = inShaderCullDistance; }
	inline VkBool32 getShaderFloat64() const { return shaderFloat64; }
	inline void setShaderFloat64(VkBool32 inShaderFloat64) { this->shaderFloat64 = inShaderFloat64; }
	inline VkBool32 getShaderInt64() const { return shaderInt64; }
	inline void setShaderInt64(VkBool32 inShaderInt64) { this->shaderInt64 = inShaderInt64; }
	inline VkBool32 getShaderInt16() const { return shaderInt16; }
	inline void setShaderInt16(VkBool32 inShaderInt16) { this->shaderInt16 = inShaderInt16; }
	inline VkBool32 getShaderResourceResidency() const { return shaderResourceResidency; }
	inline void setShaderResourceResidency(VkBool32 inShaderResourceResidency) { this->shaderResourceResidency = inShaderResourceResidency; }
	inline VkBool32 getShaderResourceMinLod() const { return shaderResourceMinLod; }
	inline void setShaderResourceMinLod(VkBool32 inShaderResourceMinLod) { this->shaderResourceMinLod = inShaderResourceMinLod; }
	inline VkBool32 getSparseBinding() const { return sparseBinding; }
	inline void setSparseBinding(VkBool32 inSparseBinding) { this->sparseBinding = inSparseBinding; }
	inline VkBool32 getSparseResidencyBuffer() const { return sparseResidencyBuffer; }
	inline void setSparseResidencyBuffer(VkBool32 inSparseResidencyBuffer) { this->sparseResidencyBuffer = inSparseResidencyBuffer; }
	inline VkBool32 getSparseResidencyImage2D() const { return sparseResidencyImage2D; }
	inline void setSparseResidencyImage2D(VkBool32 inSparseResidencyImage2D) { this->sparseResidencyImage2D = inSparseResidencyImage2D; }
	inline VkBool32 getSparseResidencyImage3D() const { return sparseResidencyImage3D; }
	inline void setSparseResidencyImage3D(VkBool32 inSparseResidencyImage3D) { this->sparseResidencyImage3D = inSparseResidencyImage3D; }
	inline VkBool32 getSparseResidency2Samples() const { return sparseResidency2Samples; }
	inline void setSparseResidency2Samples(VkBool32 inSparseResidency2Samples) { this->sparseResidency2Samples = inSparseResidency2Samples; }
	inline VkBool32 getSparseResidency4Samples() const { return sparseResidency4Samples; }
	inline void setSparseResidency4Samples(VkBool32 inSparseResidency4Samples) { this->sparseResidency4Samples = inSparseResidency4Samples; }
	inline VkBool32 getSparseResidency8Samples() const { return sparseResidency8Samples; }
	inline void setSparseResidency8Samples(VkBool32 inSparseResidency8Samples) { this->sparseResidency8Samples = inSparseResidency8Samples; }
	inline VkBool32 getSparseResidency16Samples() const { return sparseResidency16Samples; }
	inline void setSparseResidency16Samples(VkBool32 inSparseResidency16Samples) { this->sparseResidency16Samples = inSparseResidency16Samples; }
	inline VkBool32 getSparseResidencyAliased() const { return sparseResidencyAliased; }
	inline void setSparseResidencyAliased(VkBool32 inSparseResidencyAliased) { this->sparseResidencyAliased = inSparseResidencyAliased; }
	inline VkBool32 getVariableMultisampleRate() const { return variableMultisampleRate; }
	inline void setVariableMultisampleRate(VkBool32 inVariableMultisampleRate) { this->variableMultisampleRate = inVariableMultisampleRate; }
	inline VkBool32 getInheritedQueries() const { return inheritedQueries; }
	inline void setInheritedQueries(VkBool32 inInheritedQueries) { this->inheritedQueries = inInheritedQueries; }
	inline VkPhysicalDeviceFeatures& get() { return *this; }
	inline const VkPhysicalDeviceFeatures& get() const { return *this; }
};

// QueueFamilyProperties is a structure used only as a return type so only getters are defined
struct QueueFamilyProperties: private VkQueueFamilyProperties
{
	QueueFamilyProperties(){}
	QueueFamilyProperties(const VkQueueFamilyProperties& vkType): VkQueueFamilyProperties(vkType){}
	inline QueueFlags getQueueFlags() const { return (QueueFlags)queueFlags; }
	inline uint32_t getQueueCount() const { return queueCount; }
	inline uint32_t getTimestampValidBits() const { return timestampValidBits; }
	inline const Extent3D& getMinImageTransferGranularity() const { return (Extent3D&)minImageTransferGranularity; }
	inline VkQueueFamilyProperties& get() { return *this; }
	inline const VkQueueFamilyProperties& get() const { return *this; }
};

// MemoryType is a structure used only as a return type so only getters are defined
struct MemoryType: private VkMemoryType
{
	MemoryType(){}
	MemoryType(const VkMemoryType& vkType): VkMemoryType(vkType){}
	inline MemoryPropertyFlags getPropertyFlags() const { return (MemoryPropertyFlags)propertyFlags; }
	inline uint32_t getHeapIndex() const { return heapIndex; }
	inline VkMemoryType& get() { return *this; }
	inline const VkMemoryType& get() const { return *this; }
};

// MemoryHeap is a structure used only as a return type so only getters are defined
struct MemoryHeap: private VkMemoryHeap
{
	MemoryHeap(){}
	MemoryHeap(const VkMemoryHeap& vkType): VkMemoryHeap(vkType){}
	inline VkDeviceSize getSize() const { return size; }
	inline MemoryHeapFlags getFlags() const { return (MemoryHeapFlags)flags; }
	inline VkMemoryHeap& get() { return *this; }
	inline const VkMemoryHeap& get() const { return *this; }
};

// PhysicalDeviceMemoryProperties is a structure used only as a return type so only getters are defined
struct PhysicalDeviceMemoryProperties: private VkPhysicalDeviceMemoryProperties
{
	PhysicalDeviceMemoryProperties(){}
	PhysicalDeviceMemoryProperties(const VkPhysicalDeviceMemoryProperties& vkType): VkPhysicalDeviceMemoryProperties(vkType){}
	inline uint32_t getMemoryTypeCount() const { return memoryTypeCount; }
	inline const MemoryType* getMemoryTypes() const { return (MemoryType*)memoryTypes; }
	inline uint32_t getMemoryHeapCount() const { return memoryHeapCount; }
	inline const MemoryHeap* getMemoryHeaps() const { return (MemoryHeap*)memoryHeaps; }
	inline VkPhysicalDeviceMemoryProperties& get() { return *this; }
	inline const VkPhysicalDeviceMemoryProperties& get() const { return *this; }
};

// MemoryRequirements is a structure used only as a return type so only getters are defined
struct MemoryRequirements: private VkMemoryRequirements
{
	MemoryRequirements(){}
	MemoryRequirements(const VkMemoryRequirements& vkType): VkMemoryRequirements(vkType){}
	inline VkDeviceSize getSize() const { return size; }
	inline VkDeviceSize getAlignment() const { return alignment; }
	inline uint32_t getMemoryTypeBits() const { return memoryTypeBits; }
	inline VkMemoryRequirements& get() { return *this; }
	inline const VkMemoryRequirements& get() const { return *this; }
};

// SparseImageFormatProperties is a structure used only as a return type so only getters are defined
struct SparseImageFormatProperties: private VkSparseImageFormatProperties
{
	SparseImageFormatProperties(){}
	SparseImageFormatProperties(const VkSparseImageFormatProperties& vkType): VkSparseImageFormatProperties(vkType){}
	inline ImageAspectFlags getAspectMask() const { return (ImageAspectFlags)aspectMask; }
	inline const Extent3D& getImageGranularity() const { return (Extent3D&)imageGranularity; }
	inline SparseImageFormatFlags getFlags() const { return (SparseImageFormatFlags)flags; }
	inline VkSparseImageFormatProperties& get() { return *this; }
	inline const VkSparseImageFormatProperties& get() const { return *this; }
};

// SparseImageMemoryRequirements is a structure used only as a return type so only getters are defined
struct SparseImageMemoryRequirements: private VkSparseImageMemoryRequirements
{
	SparseImageMemoryRequirements(){}
	SparseImageMemoryRequirements(const VkSparseImageMemoryRequirements& vkType): VkSparseImageMemoryRequirements(vkType){}
	inline const SparseImageFormatProperties& getFormatProperties() const { return (SparseImageFormatProperties&)formatProperties; }
	inline uint32_t getImageMipTailFirstLod() const { return imageMipTailFirstLod; }
	inline VkDeviceSize getImageMipTailSize() const { return imageMipTailSize; }
	inline VkDeviceSize getImageMipTailOffset() const { return imageMipTailOffset; }
	inline VkDeviceSize getImageMipTailStride() const { return imageMipTailStride; }
	inline VkSparseImageMemoryRequirements& get() { return *this; }
	inline const VkSparseImageMemoryRequirements& get() const { return *this; }
};

// FormatProperties is a structure used only as a return type so only getters are defined
struct FormatProperties: private VkFormatProperties
{
	FormatProperties(){}
	FormatProperties(const VkFormatProperties& vkType): VkFormatProperties(vkType){}
	inline FormatFeatureFlags getLinearTilingFeatures() const { return (FormatFeatureFlags)linearTilingFeatures; }
	inline FormatFeatureFlags getOptimalTilingFeatures() const { return (FormatFeatureFlags)optimalTilingFeatures; }
	inline FormatFeatureFlags getBufferFeatures() const { return (FormatFeatureFlags)bufferFeatures; }
	inline VkFormatProperties& get() { return *this; }
	inline const VkFormatProperties& get() const { return *this; }
};

// ImageFormatProperties is a structure used only as a return type so only getters are defined
struct ImageFormatProperties: private VkImageFormatProperties
{
	ImageFormatProperties(){}
	ImageFormatProperties(const VkImageFormatProperties& vkType): VkImageFormatProperties(vkType){}
	inline const Extent3D& getMaxExtent() const { return (Extent3D&)maxExtent; }
	inline uint32_t getMaxMipLevels() const { return maxMipLevels; }
	inline uint32_t getMaxArrayLayers() const { return maxArrayLayers; }
	inline SampleCountFlags getSampleCounts() const { return (SampleCountFlags)sampleCounts; }
	inline VkDeviceSize getMaxResourceSize() const { return maxResourceSize; }
	inline VkImageFormatProperties& get() { return *this; }
	inline const VkImageFormatProperties& get() const { return *this; }
};

struct ImageSubresource: private VkImageSubresource
{
	ImageSubresource()
	{
		setAspectMask(ImageAspectFlags());
		setMipLevel(uint32_t());
		setArrayLayer(uint32_t());
	}
	ImageSubresource(const VkImageSubresource& vkType): VkImageSubresource(vkType){}
	ImageSubresource(ImageAspectFlags aspectMask, uint32_t mipLevel, uint32_t arrayLayer)
	{
		setAspectMask(aspectMask);
		setMipLevel(mipLevel);
		setArrayLayer(arrayLayer);
	}
	inline ImageAspectFlags getAspectMask() const { return (ImageAspectFlags)aspectMask; }
	inline void setAspectMask(ImageAspectFlags inAspectMask) { this->aspectMask = (VkImageAspectFlags)inAspectMask; }
	inline uint32_t getMipLevel() const { return mipLevel; }
	inline void setMipLevel(uint32_t inMipLevel) { this->mipLevel = inMipLevel; }
	inline uint32_t getArrayLayer() const { return arrayLayer; }
	inline void setArrayLayer(uint32_t inArrayLayer) { this->arrayLayer = inArrayLayer; }
	inline VkImageSubresource& get() { return *this; }
	inline const VkImageSubresource& get() const { return *this; }
};

struct ImageSubresourceLayers: private VkImageSubresourceLayers
{
	ImageSubresourceLayers()
	{
		setAspectMask(ImageAspectFlags::e_COLOR_BIT);
		setMipLevel(0);
		setBaseArrayLayer(0);
		setLayerCount(1);
	}
	ImageSubresourceLayers(const VkImageSubresourceLayers& vkType): VkImageSubresourceLayers(vkType){}
	ImageSubresourceLayers(ImageAspectFlags aspectMask, uint32_t mipLevel, uint32_t baseArrayLayer, uint32_t layerCount)
	{
		setAspectMask(aspectMask);
		setMipLevel(mipLevel);
		setBaseArrayLayer(baseArrayLayer);
		setLayerCount(layerCount);
	}
	inline ImageAspectFlags getAspectMask() const { return (ImageAspectFlags)aspectMask; }
	inline void setAspectMask(ImageAspectFlags inAspectMask) { this->aspectMask = (VkImageAspectFlags)inAspectMask; }
	inline uint32_t getMipLevel() const { return mipLevel; }
	inline void setMipLevel(uint32_t inMipLevel) { this->mipLevel = inMipLevel; }
	inline uint32_t getBaseArrayLayer() const { return baseArrayLayer; }
	inline void setBaseArrayLayer(uint32_t inBaseArrayLayer) { this->baseArrayLayer = inBaseArrayLayer; }
	inline uint32_t getLayerCount() const { return layerCount; }
	inline void setLayerCount(uint32_t inLayerCount) { this->layerCount = inLayerCount; }
	inline VkImageSubresourceLayers& get() { return *this; }
	inline const VkImageSubresourceLayers& get() const { return *this; }
};

struct ImageSubresourceRange: private VkImageSubresourceRange
{
	ImageSubresourceRange()
	{
		setAspectMask(ImageAspectFlags::e_MAX_ENUM);
		setBaseMipLevel(0);
		setLevelCount(1);
		setBaseArrayLayer(0);
		setLayerCount(1);
	}
	ImageSubresourceRange(const VkImageSubresourceRange& vkType): VkImageSubresourceRange(vkType){}
	ImageSubresourceRange(ImageAspectFlags aspectMask, uint32_t baseMipLevel = 0, uint32_t levelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1)
	{
		setAspectMask(aspectMask);
		setBaseMipLevel(baseMipLevel);
		setLevelCount(levelCount);
		setBaseArrayLayer(baseArrayLayer);
		setLayerCount(layerCount);
	}
	inline ImageAspectFlags getAspectMask() const { return (ImageAspectFlags)aspectMask; }
	inline void setAspectMask(ImageAspectFlags inAspectMask) { this->aspectMask = (VkImageAspectFlags)inAspectMask; }
	inline uint32_t getBaseMipLevel() const { return baseMipLevel; }
	inline void setBaseMipLevel(uint32_t inBaseMipLevel) { this->baseMipLevel = inBaseMipLevel; }
	inline uint32_t getLevelCount() const { return levelCount; }
	inline void setLevelCount(uint32_t inLevelCount) { this->levelCount = inLevelCount; }
	inline uint32_t getBaseArrayLayer() const { return baseArrayLayer; }
	inline void setBaseArrayLayer(uint32_t inBaseArrayLayer) { this->baseArrayLayer = inBaseArrayLayer; }
	inline uint32_t getLayerCount() const { return layerCount; }
	inline void setLayerCount(uint32_t inLayerCount) { this->layerCount = inLayerCount; }
	inline VkImageSubresourceRange& get() { return *this; }
	inline const VkImageSubresourceRange& get() const { return *this; }
};

// SubresourceLayout is a structure used only as a return type so only getters are defined
struct SubresourceLayout: private VkSubresourceLayout
{
	SubresourceLayout(){}
	SubresourceLayout(const VkSubresourceLayout& vkType): VkSubresourceLayout(vkType){}
	inline VkDeviceSize getOffset() const { return offset; }
	inline VkDeviceSize getSize() const { return size; }
	inline VkDeviceSize getRowPitch() const { return rowPitch; }
	inline VkDeviceSize getArrayPitch() const { return arrayPitch; }
	inline VkDeviceSize getDepthPitch() const { return depthPitch; }
	inline VkSubresourceLayout& get() { return *this; }
	inline const VkSubresourceLayout& get() const { return *this; }
};

struct BufferCopy: private VkBufferCopy
{
	BufferCopy()
	{
		setSrcOffset(VkDeviceSize());
		setDstOffset(VkDeviceSize());
		setSize(VkDeviceSize());
	}
	BufferCopy(const VkBufferCopy& vkType): VkBufferCopy(vkType){}
	BufferCopy(VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size)
	{
		setSrcOffset(srcOffset);
		setDstOffset(dstOffset);
		setSize(size);
	}
	inline VkDeviceSize getSrcOffset() const { return srcOffset; }
	inline void setSrcOffset(VkDeviceSize inSrcOffset) { this->srcOffset = inSrcOffset; }
	inline VkDeviceSize getDstOffset() const { return dstOffset; }
	inline void setDstOffset(VkDeviceSize inDstOffset) { this->dstOffset = inDstOffset; }
	inline VkDeviceSize getSize() const { return size; }
	inline void setSize(VkDeviceSize inSize) { this->size = inSize; }
	inline VkBufferCopy& get() { return *this; }
	inline const VkBufferCopy& get() const { return *this; }
};

struct ImageCopy: private VkImageCopy
{
	ImageCopy()
	{
		setSrcSubresource(ImageSubresourceLayers());
		setSrcOffset(Offset3D());
		setDstSubresource(ImageSubresourceLayers());
		setDstOffset(Offset3D());
		setExtent(Extent3D());
	}
	ImageCopy(const VkImageCopy& vkType): VkImageCopy(vkType){}
	ImageCopy(const ImageSubresourceLayers& srcSubresource, const Offset3D& srcOffset, const ImageSubresourceLayers& dstSubresource, const Offset3D& dstOffset, const Extent3D& extent)
	{
		setSrcSubresource(srcSubresource);
		setSrcOffset(srcOffset);
		setDstSubresource(dstSubresource);
		setDstOffset(dstOffset);
		setExtent(extent);
	}
	inline const ImageSubresourceLayers& getSrcSubresource() const { return (ImageSubresourceLayers&)srcSubresource; }
	inline void setSrcSubresource(const ImageSubresourceLayers& inSrcSubresource) { memcpy(&this->srcSubresource, &inSrcSubresource, sizeof(this->srcSubresource)); }
	inline const Offset3D& getSrcOffset() const { return (Offset3D&)srcOffset; }
	inline void setSrcOffset(const Offset3D& inSrcOffset) { memcpy(&this->srcOffset, &inSrcOffset, sizeof(this->srcOffset)); }
	inline const ImageSubresourceLayers& getDstSubresource() const { return (ImageSubresourceLayers&)dstSubresource; }
	inline void setDstSubresource(const ImageSubresourceLayers& inDstSubresource) { memcpy(&this->dstSubresource, &inDstSubresource, sizeof(this->dstSubresource)); }
	inline const Offset3D& getDstOffset() const { return (Offset3D&)dstOffset; }
	inline void setDstOffset(const Offset3D& inDstOffset) { memcpy(&this->dstOffset, &inDstOffset, sizeof(this->dstOffset)); }
	inline const Extent3D& getExtent() const { return (Extent3D&)extent; }
	inline void setExtent(const Extent3D& inExtent) { memcpy(&this->extent, &inExtent, sizeof(this->extent)); }
	inline VkImageCopy& get() { return *this; }
	inline const VkImageCopy& get() const { return *this; }
};

struct ImageBlit: private VkImageBlit
{
	ImageBlit()
	{
		setSrcSubresource(ImageSubresourceLayers());
		memset(srcOffsets, 0, sizeof(srcOffsets));
		setDstSubresource(ImageSubresourceLayers());
		memset(dstOffsets, 0, sizeof(dstOffsets));
	}
	ImageBlit(const VkImageBlit& vkType): VkImageBlit(vkType){}
	ImageBlit(const ImageSubresourceLayers& srcSubresource, const Offset3D* srcOffsets, const ImageSubresourceLayers& dstSubresource, const Offset3D* dstOffsets)
	{
		setSrcSubresource(srcSubresource);
		setSrcOffsets(srcOffsets);
		setDstSubresource(dstSubresource);
		setDstOffsets(dstOffsets);
	}
	inline const ImageSubresourceLayers& getSrcSubresource() const { return (ImageSubresourceLayers&)srcSubresource; }
	inline void setSrcSubresource(const ImageSubresourceLayers& inSrcSubresource) { memcpy(&this->srcSubresource, &inSrcSubresource, sizeof(this->srcSubresource)); }
	inline const Offset3D* getSrcOffsets() const { return (Offset3D*)srcOffsets; }
	inline void setSrcOffsets(const Offset3D* inSrcOffsets) { memcpy(this->srcOffsets, inSrcOffsets, sizeof(this->srcOffsets)); }
	inline const ImageSubresourceLayers& getDstSubresource() const { return (ImageSubresourceLayers&)dstSubresource; }
	inline void setDstSubresource(const ImageSubresourceLayers& inDstSubresource) { memcpy(&this->dstSubresource, &inDstSubresource, sizeof(this->dstSubresource)); }
	inline const Offset3D* getDstOffsets() const { return (Offset3D*)dstOffsets; }
	inline void setDstOffsets(const Offset3D* inDstOffsets) { memcpy(this->dstOffsets, inDstOffsets, sizeof(this->dstOffsets)); }
	inline VkImageBlit& get() { return *this; }
	inline const VkImageBlit& get() const { return *this; }
};

struct BufferImageCopy: private VkBufferImageCopy
{
	BufferImageCopy()
	{
		setBufferOffset(VkDeviceSize());
		setBufferRowLength(uint32_t());
		setBufferImageHeight(uint32_t());
		setImageSubresource(ImageSubresourceLayers());
		setImageOffset(Offset3D());
		setImageExtent(Extent3D());
	}
	BufferImageCopy(const VkBufferImageCopy& vkType): VkBufferImageCopy(vkType){}
	BufferImageCopy(VkDeviceSize bufferOffset, uint32_t bufferRowLength, uint32_t bufferImageHeight, const ImageSubresourceLayers& imageSubresource, const Offset3D& imageOffset, const Extent3D& imageExtent)
	{
		setBufferOffset(bufferOffset);
		setBufferRowLength(bufferRowLength);
		setBufferImageHeight(bufferImageHeight);
		setImageSubresource(imageSubresource);
		setImageOffset(imageOffset);
		setImageExtent(imageExtent);
	}
	inline VkDeviceSize getBufferOffset() const { return bufferOffset; }
	inline void setBufferOffset(VkDeviceSize inBufferOffset) { this->bufferOffset = inBufferOffset; }
	inline uint32_t getBufferRowLength() const { return bufferRowLength; }
	inline void setBufferRowLength(uint32_t inBufferRowLength) { this->bufferRowLength = inBufferRowLength; }
	inline uint32_t getBufferImageHeight() const { return bufferImageHeight; }
	inline void setBufferImageHeight(uint32_t inBufferImageHeight) { this->bufferImageHeight = inBufferImageHeight; }
	inline const ImageSubresourceLayers& getImageSubresource() const { return (ImageSubresourceLayers&)imageSubresource; }
	inline void setImageSubresource(const ImageSubresourceLayers& inImageSubresource) { memcpy(&this->imageSubresource, &inImageSubresource, sizeof(this->imageSubresource)); }
	inline const Offset3D& getImageOffset() const { return (Offset3D&)imageOffset; }
	inline void setImageOffset(const Offset3D& inImageOffset) { memcpy(&this->imageOffset, &inImageOffset, sizeof(this->imageOffset)); }
	inline const Extent3D& getImageExtent() const { return (Extent3D&)imageExtent; }
	inline void setImageExtent(const Extent3D& inImageExtent) { memcpy(&this->imageExtent, &inImageExtent, sizeof(this->imageExtent)); }
	inline VkBufferImageCopy& get() { return *this; }
	inline const VkBufferImageCopy& get() const { return *this; }
};

struct ImageResolve: private VkImageResolve
{
	ImageResolve()
	{
		setSrcSubresource(ImageSubresourceLayers());
		setSrcOffset(Offset3D());
		setDstSubresource(ImageSubresourceLayers());
		setDstOffset(Offset3D());
		setExtent(Extent3D());
	}
	ImageResolve(const VkImageResolve& vkType): VkImageResolve(vkType){}
	ImageResolve(const ImageSubresourceLayers& srcSubresource, const Offset3D& srcOffset, const ImageSubresourceLayers& dstSubresource, const Offset3D& dstOffset, const Extent3D& extent)
	{
		setSrcSubresource(srcSubresource);
		setSrcOffset(srcOffset);
		setDstSubresource(dstSubresource);
		setDstOffset(dstOffset);
		setExtent(extent);
	}
	inline const ImageSubresourceLayers& getSrcSubresource() const { return (ImageSubresourceLayers&)srcSubresource; }
	inline void setSrcSubresource(const ImageSubresourceLayers& inSrcSubresource) { memcpy(&this->srcSubresource, &inSrcSubresource, sizeof(this->srcSubresource)); }
	inline const Offset3D& getSrcOffset() const { return (Offset3D&)srcOffset; }
	inline void setSrcOffset(const Offset3D& inSrcOffset) { memcpy(&this->srcOffset, &inSrcOffset, sizeof(this->srcOffset)); }
	inline const ImageSubresourceLayers& getDstSubresource() const { return (ImageSubresourceLayers&)dstSubresource; }
	inline void setDstSubresource(const ImageSubresourceLayers& inDstSubresource) { memcpy(&this->dstSubresource, &inDstSubresource, sizeof(this->dstSubresource)); }
	inline const Offset3D& getDstOffset() const { return (Offset3D&)dstOffset; }
	inline void setDstOffset(const Offset3D& inDstOffset) { memcpy(&this->dstOffset, &inDstOffset, sizeof(this->dstOffset)); }
	inline const Extent3D& getExtent() const { return (Extent3D&)extent; }
	inline void setExtent(const Extent3D& inExtent) { memcpy(&this->extent, &inExtent, sizeof(this->extent)); }
	inline VkImageResolve& get() { return *this; }
	inline const VkImageResolve& get() const { return *this; }
};

struct DescriptorSetLayoutBinding: private VkDescriptorSetLayoutBinding
{
	DescriptorSetLayoutBinding()
	{
		setBinding(uint32_t());
		setDescriptorType(DescriptorType());
		setDescriptorCount(uint32_t());
		setStageFlags(ShaderStageFlags());
		setPImmutableSamplers(nullptr);
	}
	DescriptorSetLayoutBinding(const VkDescriptorSetLayoutBinding& vkType): VkDescriptorSetLayoutBinding(vkType){}
	DescriptorSetLayoutBinding(uint32_t binding, DescriptorType descriptorType, uint32_t descriptorCount, ShaderStageFlags stageFlags, VkSampler* pImmutableSamplers)
	{
		setBinding(binding);
		setDescriptorType(descriptorType);
		setDescriptorCount(descriptorCount);
		setStageFlags(stageFlags);
		setPImmutableSamplers(pImmutableSamplers);
	}
	inline uint32_t getBinding() const { return binding; }
	inline void setBinding(uint32_t inBinding) { this->binding = inBinding; }
	inline DescriptorType getDescriptorType() const { return (DescriptorType)descriptorType; }
	inline void setDescriptorType(DescriptorType inDescriptorType) { this->descriptorType = (VkDescriptorType)inDescriptorType; }
	inline uint32_t getDescriptorCount() const { return descriptorCount; }
	inline void setDescriptorCount(uint32_t inDescriptorCount) { this->descriptorCount = inDescriptorCount; }
	inline ShaderStageFlags getStageFlags() const { return (ShaderStageFlags)stageFlags; }
	inline void setStageFlags(ShaderStageFlags inStageFlags) { this->stageFlags = (VkShaderStageFlags)inStageFlags; }
	inline const VkSampler* getPImmutableSamplers() const { return pImmutableSamplers; }
	inline void setPImmutableSamplers(VkSampler* inPImmutableSamplers) { this->pImmutableSamplers = inPImmutableSamplers; }
	inline VkDescriptorSetLayoutBinding& get() { return *this; }
	inline const VkDescriptorSetLayoutBinding& get() const { return *this; }
};

struct DescriptorPoolSize: private VkDescriptorPoolSize
{
	DescriptorPoolSize()
	{
		setType(DescriptorType());
		setDescriptorCount(uint32_t());
	}
	DescriptorPoolSize(const VkDescriptorPoolSize& vkType): VkDescriptorPoolSize(vkType){}
	DescriptorPoolSize(DescriptorType type, uint32_t descriptorCount)
	{
		setType(type);
		setDescriptorCount(descriptorCount);
	}
	inline DescriptorType getType() const { return (DescriptorType)type; }
	inline void setType(DescriptorType inType) { this->type = (VkDescriptorType)inType; }
	inline uint32_t getDescriptorCount() const { return descriptorCount; }
	inline void setDescriptorCount(uint32_t inDescriptorCount) { this->descriptorCount = inDescriptorCount; }
	inline VkDescriptorPoolSize& get() { return *this; }
	inline const VkDescriptorPoolSize& get() const { return *this; }
};

struct SpecializationMapEntry: private VkSpecializationMapEntry
{
	SpecializationMapEntry()
	{
		setConstantID(uint32_t());
		setOffset(uint32_t());
		setSize(size_t());
	}
	SpecializationMapEntry(const VkSpecializationMapEntry& vkType): VkSpecializationMapEntry(vkType){}
	SpecializationMapEntry(uint32_t constantID, uint32_t offset, size_t size)
	{
		setConstantID(constantID);
		setOffset(offset);
		setSize(size);
	}
	inline uint32_t getConstantID() const { return constantID; }
	inline void setConstantID(uint32_t inConstantID) { this->constantID = inConstantID; }
	inline uint32_t getOffset() const { return offset; }
	inline void setOffset(uint32_t inOffset) { this->offset = inOffset; }
	inline size_t getSize() const { return size; }
	inline void setSize(size_t inSize) { this->size = inSize; }
	inline VkSpecializationMapEntry& get() { return *this; }
	inline const VkSpecializationMapEntry& get() const { return *this; }
};

struct SpecializationInfo: private VkSpecializationInfo
{
	SpecializationInfo()
	{
		setMapEntryCount(uint32_t());
		setPMapEntries(nullptr);
		setDataSize(size_t());
		setPData(nullptr);
	}
	SpecializationInfo(const VkSpecializationInfo& vkType): VkSpecializationInfo(vkType){}
	SpecializationInfo(uint32_t mapEntryCount, SpecializationMapEntry* pMapEntries, size_t dataSize, void* pData)
	{
		setMapEntryCount(mapEntryCount);
		setPMapEntries(pMapEntries);
		setDataSize(dataSize);
		setPData(pData);
	}
	inline uint32_t getMapEntryCount() const { return mapEntryCount; }
	inline void setMapEntryCount(uint32_t inMapEntryCount) { this->mapEntryCount = inMapEntryCount; }
	inline const SpecializationMapEntry* getPMapEntries() const { return (SpecializationMapEntry*)pMapEntries; }
	inline void setPMapEntries(SpecializationMapEntry* inPMapEntries) { this->pMapEntries = (VkSpecializationMapEntry*)inPMapEntries; }
	inline size_t getDataSize() const { return dataSize; }
	inline void setDataSize(size_t inDataSize) { this->dataSize = inDataSize; }
	inline const void* getPData() const { return pData; }
	inline void setPData(void* inPData) { this->pData = inPData; }
	inline VkSpecializationInfo& get() { return *this; }
	inline const VkSpecializationInfo& get() const { return *this; }
};

struct VertexInputBindingDescription: private VkVertexInputBindingDescription
{
	VertexInputBindingDescription()
	{
		setBinding(0);
		setStride(0);
		setInputRate(VertexInputRate::e_VERTEX);
	}
	VertexInputBindingDescription(const VkVertexInputBindingDescription& vkType): VkVertexInputBindingDescription(vkType){}
	VertexInputBindingDescription(uint32_t binding, uint32_t stride, VertexInputRate inputRate = VertexInputRate::e_VERTEX)
	{
		setBinding(binding);
		setStride(stride);
		setInputRate(inputRate);
	}
	inline uint32_t getBinding() const { return binding; }
	inline void setBinding(uint32_t inBinding) { this->binding = inBinding; }
	inline uint32_t getStride() const { return stride; }
	inline void setStride(uint32_t inStride) { this->stride = inStride; }
	inline VertexInputRate getInputRate() const { return (VertexInputRate)inputRate; }
	inline void setInputRate(VertexInputRate inInputRate) { this->inputRate = (VkVertexInputRate)inInputRate; }
	inline VkVertexInputBindingDescription& get() { return *this; }
	inline const VkVertexInputBindingDescription& get() const { return *this; }
};

struct VertexInputAttributeDescription: private VkVertexInputAttributeDescription
{
	VertexInputAttributeDescription()
	{
		setLocation(uint32_t());
		setBinding(uint32_t());
		setFormat(Format());
		setOffset(uint32_t());
	}
	VertexInputAttributeDescription(const VkVertexInputAttributeDescription& vkType): VkVertexInputAttributeDescription(vkType){}
	VertexInputAttributeDescription(uint32_t location, uint32_t binding, Format format, uint32_t offset)
	{
		setLocation(location);
		setBinding(binding);
		setFormat(format);
		setOffset(offset);
	}
	inline uint32_t getLocation() const { return location; }
	inline void setLocation(uint32_t inLocation) { this->location = inLocation; }
	inline uint32_t getBinding() const { return binding; }
	inline void setBinding(uint32_t inBinding) { this->binding = inBinding; }
	inline Format getFormat() const { return (Format)format; }
	inline void setFormat(Format inFormat) { this->format = (VkFormat)inFormat; }
	inline uint32_t getOffset() const { return offset; }
	inline void setOffset(uint32_t inOffset) { this->offset = inOffset; }
	inline VkVertexInputAttributeDescription& get() { return *this; }
	inline const VkVertexInputAttributeDescription& get() const { return *this; }
};

struct PipelineColorBlendAttachmentState: private VkPipelineColorBlendAttachmentState
{
	PipelineColorBlendAttachmentState()
	{
		setBlendEnable(false);
		setSrcColorBlendFactor(BlendFactor::e_ONE);
		setDstColorBlendFactor(BlendFactor::e_ZERO);
		setColorBlendOp(BlendOp::e_ADD);
		setSrcAlphaBlendFactor(BlendFactor::e_ONE);
		setDstAlphaBlendFactor(BlendFactor::e_ZERO);
		setAlphaBlendOp(BlendOp::e_ADD);
		setColorWriteMask(ColorComponentFlags::e_ALL_BITS);
	}
	PipelineColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState& vkType): VkPipelineColorBlendAttachmentState(vkType){}
	PipelineColorBlendAttachmentState(VkBool32 blendEnable, BlendFactor srcColorBlendFactor = BlendFactor::e_ONE, BlendFactor dstColorBlendFactor = BlendFactor::e_ZERO, BlendOp colorBlendOp = BlendOp::e_ADD, BlendFactor srcAlphaBlendFactor = BlendFactor::e_ONE, BlendFactor dstAlphaBlendFactor = BlendFactor::e_ZERO, BlendOp alphaBlendOp = BlendOp::e_ADD, ColorComponentFlags colorWriteMask = ColorComponentFlags::e_ALL_BITS)
	{
		setBlendEnable(blendEnable);
		setSrcColorBlendFactor(srcColorBlendFactor);
		setDstColorBlendFactor(dstColorBlendFactor);
		setColorBlendOp(colorBlendOp);
		setSrcAlphaBlendFactor(srcAlphaBlendFactor);
		setDstAlphaBlendFactor(dstAlphaBlendFactor);
		setAlphaBlendOp(alphaBlendOp);
		setColorWriteMask(colorWriteMask);
	}
	inline VkBool32 getBlendEnable() const { return blendEnable; }
	inline void setBlendEnable(VkBool32 inBlendEnable) { this->blendEnable = inBlendEnable; }
	inline BlendFactor getSrcColorBlendFactor() const { return (BlendFactor)srcColorBlendFactor; }
	inline void setSrcColorBlendFactor(BlendFactor inSrcColorBlendFactor) { this->srcColorBlendFactor = (VkBlendFactor)inSrcColorBlendFactor; }
	inline BlendFactor getDstColorBlendFactor() const { return (BlendFactor)dstColorBlendFactor; }
	inline void setDstColorBlendFactor(BlendFactor inDstColorBlendFactor) { this->dstColorBlendFactor = (VkBlendFactor)inDstColorBlendFactor; }
	inline BlendOp getColorBlendOp() const { return (BlendOp)colorBlendOp; }
	inline void setColorBlendOp(BlendOp inColorBlendOp) { this->colorBlendOp = (VkBlendOp)inColorBlendOp; }
	inline BlendFactor getSrcAlphaBlendFactor() const { return (BlendFactor)srcAlphaBlendFactor; }
	inline void setSrcAlphaBlendFactor(BlendFactor inSrcAlphaBlendFactor) { this->srcAlphaBlendFactor = (VkBlendFactor)inSrcAlphaBlendFactor; }
	inline BlendFactor getDstAlphaBlendFactor() const { return (BlendFactor)dstAlphaBlendFactor; }
	inline void setDstAlphaBlendFactor(BlendFactor inDstAlphaBlendFactor) { this->dstAlphaBlendFactor = (VkBlendFactor)inDstAlphaBlendFactor; }
	inline BlendOp getAlphaBlendOp() const { return (BlendOp)alphaBlendOp; }
	inline void setAlphaBlendOp(BlendOp inAlphaBlendOp) { this->alphaBlendOp = (VkBlendOp)inAlphaBlendOp; }
	inline ColorComponentFlags getColorWriteMask() const { return (ColorComponentFlags)colorWriteMask; }
	inline void setColorWriteMask(ColorComponentFlags inColorWriteMask) { this->colorWriteMask = (VkColorComponentFlags)inColorWriteMask; }
	inline VkPipelineColorBlendAttachmentState& get() { return *this; }
	inline const VkPipelineColorBlendAttachmentState& get() const { return *this; }
};

struct StencilOpState: private VkStencilOpState
{
	StencilOpState()
	{
		setFailOp(StencilOp::e_KEEP);
		setPassOp(StencilOp::e_KEEP);
		setDepthFailOp(StencilOp::e_KEEP);
		setCompareOp(CompareOp::e_ALWAYS);
		setCompareMask(0xff);
		setWriteMask(0xff);
		setReference(0);
	}
	StencilOpState(const VkStencilOpState& vkType): VkStencilOpState(vkType){}
	StencilOpState(StencilOp failOp, StencilOp passOp, StencilOp depthFailOp, CompareOp compareOp, uint32_t compareMask, uint32_t writeMask, uint32_t reference)
	{
		setFailOp(failOp);
		setPassOp(passOp);
		setDepthFailOp(depthFailOp);
		setCompareOp(compareOp);
		setCompareMask(compareMask);
		setWriteMask(writeMask);
		setReference(reference);
	}
	inline StencilOp getFailOp() const { return (StencilOp)failOp; }
	inline void setFailOp(StencilOp inFailOp) { this->failOp = (VkStencilOp)inFailOp; }
	inline StencilOp getPassOp() const { return (StencilOp)passOp; }
	inline void setPassOp(StencilOp inPassOp) { this->passOp = (VkStencilOp)inPassOp; }
	inline StencilOp getDepthFailOp() const { return (StencilOp)depthFailOp; }
	inline void setDepthFailOp(StencilOp inDepthFailOp) { this->depthFailOp = (VkStencilOp)inDepthFailOp; }
	inline CompareOp getCompareOp() const { return (CompareOp)compareOp; }
	inline void setCompareOp(CompareOp inCompareOp) { this->compareOp = (VkCompareOp)inCompareOp; }
	inline uint32_t getCompareMask() const { return compareMask; }
	inline void setCompareMask(uint32_t inCompareMask) { this->compareMask = inCompareMask; }
	inline uint32_t getWriteMask() const { return writeMask; }
	inline void setWriteMask(uint32_t inWriteMask) { this->writeMask = inWriteMask; }
	inline uint32_t getReference() const { return reference; }
	inline void setReference(uint32_t inReference) { this->reference = inReference; }
	inline VkStencilOpState& get() { return *this; }
	inline const VkStencilOpState& get() const { return *this; }
};

struct PushConstantRange: private VkPushConstantRange
{
	PushConstantRange()
	{
		setStageFlags(ShaderStageFlags::e_ALL);
		setOffset(0);
		setSize(0);
	}
	PushConstantRange(const VkPushConstantRange& vkType): VkPushConstantRange(vkType){}
	PushConstantRange(ShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
	{
		setStageFlags(stageFlags);
		setOffset(offset);
		setSize(size);
	}
	inline ShaderStageFlags getStageFlags() const { return (ShaderStageFlags)stageFlags; }
	inline void setStageFlags(ShaderStageFlags inStageFlags) { this->stageFlags = (VkShaderStageFlags)inStageFlags; }
	inline uint32_t getOffset() const { return offset; }
	inline void setOffset(uint32_t inOffset) { this->offset = inOffset; }
	inline uint32_t getSize() const { return size; }
	inline void setSize(uint32_t inSize) { this->size = inSize; }
	inline VkPushConstantRange& get() { return *this; }
	inline const VkPushConstantRange& get() const { return *this; }
};

struct ClearDepthStencilValue: private VkClearDepthStencilValue
{
	ClearDepthStencilValue()
	{
		setDepth(float());
		setStencil(uint32_t());
	}
	ClearDepthStencilValue(const VkClearDepthStencilValue& vkType): VkClearDepthStencilValue(vkType){}
	ClearDepthStencilValue(float depth, uint32_t stencil)
	{
		setDepth(depth);
		setStencil(stencil);
	}
	inline float getDepth() const { return depth; }
	inline void setDepth(float inDepth) { this->depth = inDepth; }
	inline uint32_t getStencil() const { return stencil; }
	inline void setStencil(uint32_t inStencil) { this->stencil = inStencil; }
	inline VkClearDepthStencilValue& get() { return *this; }
	inline const VkClearDepthStencilValue& get() const { return *this; }
};

struct AttachmentReference: private VkAttachmentReference
{
	AttachmentReference()
	{
		setAttachment(static_cast<uint32_t>(-1));
		setLayout(ImageLayout::e_UNDEFINED);
	}
	AttachmentReference(const VkAttachmentReference& vkType): VkAttachmentReference(vkType){}
	AttachmentReference(uint32_t attachment, ImageLayout layout)
	{
		setAttachment(attachment);
		setLayout(layout);
	}
	inline uint32_t getAttachment() const { return attachment; }
	inline void setAttachment(uint32_t inAttachment) { this->attachment = inAttachment; }
	inline ImageLayout getLayout() const { return (ImageLayout)layout; }
	inline void setLayout(ImageLayout inLayout) { this->layout = (VkImageLayout)inLayout; }
	inline VkAttachmentReference& get() { return *this; }
	inline const VkAttachmentReference& get() const { return *this; }
};

struct SubpassDependency: private VkSubpassDependency
{
	SubpassDependency()
	{
		setSrcSubpass(uint32_t());
		setDstSubpass(uint32_t());
		setSrcStageMask(PipelineStageFlags());
		setDstStageMask(PipelineStageFlags());
		setSrcAccessMask(AccessFlags());
		setDstAccessMask(AccessFlags());
		setDependencyFlags(DependencyFlags());
	}
	SubpassDependency(const VkSubpassDependency& vkType): VkSubpassDependency(vkType){}
	SubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, PipelineStageFlags srcStageMask, PipelineStageFlags dstStageMask, AccessFlags srcAccessMask, AccessFlags dstAccessMask, DependencyFlags dependencyFlags)
	{
		setSrcSubpass(srcSubpass);
		setDstSubpass(dstSubpass);
		setSrcStageMask(srcStageMask);
		setDstStageMask(dstStageMask);
		setSrcAccessMask(srcAccessMask);
		setDstAccessMask(dstAccessMask);
		setDependencyFlags(dependencyFlags);
	}
	inline uint32_t getSrcSubpass() const { return srcSubpass; }
	inline void setSrcSubpass(uint32_t inSrcSubpass) { this->srcSubpass = inSrcSubpass; }
	inline uint32_t getDstSubpass() const { return dstSubpass; }
	inline void setDstSubpass(uint32_t inDstSubpass) { this->dstSubpass = inDstSubpass; }
	inline PipelineStageFlags getSrcStageMask() const { return (PipelineStageFlags)srcStageMask; }
	inline void setSrcStageMask(PipelineStageFlags inSrcStageMask) { this->srcStageMask = (VkPipelineStageFlags)inSrcStageMask; }
	inline PipelineStageFlags getDstStageMask() const { return (PipelineStageFlags)dstStageMask; }
	inline void setDstStageMask(PipelineStageFlags inDstStageMask) { this->dstStageMask = (VkPipelineStageFlags)inDstStageMask; }
	inline AccessFlags getSrcAccessMask() const { return (AccessFlags)srcAccessMask; }
	inline void setSrcAccessMask(AccessFlags inSrcAccessMask) { this->srcAccessMask = (VkAccessFlags)inSrcAccessMask; }
	inline AccessFlags getDstAccessMask() const { return (AccessFlags)dstAccessMask; }
	inline void setDstAccessMask(AccessFlags inDstAccessMask) { this->dstAccessMask = (VkAccessFlags)inDstAccessMask; }
	inline DependencyFlags getDependencyFlags() const { return (DependencyFlags)dependencyFlags; }
	inline void setDependencyFlags(DependencyFlags inDependencyFlags) { this->dependencyFlags = (VkDependencyFlags)inDependencyFlags; }
	inline VkSubpassDependency& get() { return *this; }
	inline const VkSubpassDependency& get() const { return *this; }
};

struct DrawIndirectCommand: private VkDrawIndirectCommand
{
	DrawIndirectCommand()
	{
		setVertexCount(uint32_t());
		setInstanceCount(uint32_t());
		setFirstVertex(uint32_t());
		setFirstInstance(uint32_t());
	}
	DrawIndirectCommand(const VkDrawIndirectCommand& vkType): VkDrawIndirectCommand(vkType){}
	DrawIndirectCommand(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		setVertexCount(vertexCount);
		setInstanceCount(instanceCount);
		setFirstVertex(firstVertex);
		setFirstInstance(firstInstance);
	}
	inline uint32_t getVertexCount() const { return vertexCount; }
	inline void setVertexCount(uint32_t inVertexCount) { this->vertexCount = inVertexCount; }
	inline uint32_t getInstanceCount() const { return instanceCount; }
	inline void setInstanceCount(uint32_t inInstanceCount) { this->instanceCount = inInstanceCount; }
	inline uint32_t getFirstVertex() const { return firstVertex; }
	inline void setFirstVertex(uint32_t inFirstVertex) { this->firstVertex = inFirstVertex; }
	inline uint32_t getFirstInstance() const { return firstInstance; }
	inline void setFirstInstance(uint32_t inFirstInstance) { this->firstInstance = inFirstInstance; }
	inline VkDrawIndirectCommand& get() { return *this; }
	inline const VkDrawIndirectCommand& get() const { return *this; }
};

struct DrawIndexedIndirectCommand: private VkDrawIndexedIndirectCommand
{
	DrawIndexedIndirectCommand()
	{
		setIndexCount(uint32_t());
		setInstanceCount(uint32_t());
		setFirstIndex(uint32_t());
		setVertexOffset(int32_t());
		setFirstInstance(uint32_t());
	}
	DrawIndexedIndirectCommand(const VkDrawIndexedIndirectCommand& vkType): VkDrawIndexedIndirectCommand(vkType){}
	DrawIndexedIndirectCommand(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
	{
		setIndexCount(indexCount);
		setInstanceCount(instanceCount);
		setFirstIndex(firstIndex);
		setVertexOffset(vertexOffset);
		setFirstInstance(firstInstance);
	}
	inline uint32_t getIndexCount() const { return indexCount; }
	inline void setIndexCount(uint32_t inIndexCount) { this->indexCount = inIndexCount; }
	inline uint32_t getInstanceCount() const { return instanceCount; }
	inline void setInstanceCount(uint32_t inInstanceCount) { this->instanceCount = inInstanceCount; }
	inline uint32_t getFirstIndex() const { return firstIndex; }
	inline void setFirstIndex(uint32_t inFirstIndex) { this->firstIndex = inFirstIndex; }
	inline int32_t getVertexOffset() const { return vertexOffset; }
	inline void setVertexOffset(int32_t inVertexOffset) { this->vertexOffset = inVertexOffset; }
	inline uint32_t getFirstInstance() const { return firstInstance; }
	inline void setFirstInstance(uint32_t inFirstInstance) { this->firstInstance = inFirstInstance; }
	inline VkDrawIndexedIndirectCommand& get() { return *this; }
	inline const VkDrawIndexedIndirectCommand& get() const { return *this; }
};

struct DispatchIndirectCommand: private VkDispatchIndirectCommand
{
	DispatchIndirectCommand()
	{
		setX(uint32_t());
		setY(uint32_t());
		setZ(uint32_t());
	}
	DispatchIndirectCommand(const VkDispatchIndirectCommand& vkType): VkDispatchIndirectCommand(vkType){}
	DispatchIndirectCommand(uint32_t x, uint32_t y, uint32_t z)
	{
		setX(x);
		setY(y);
		setZ(z);
	}
	inline uint32_t getX() const { return x; }
	inline void setX(uint32_t inX) { this->x = inX; }
	inline uint32_t getY() const { return y; }
	inline void setY(uint32_t inY) { this->y = inY; }
	inline uint32_t getZ() const { return z; }
	inline void setZ(uint32_t inZ) { this->z = inZ; }
	inline VkDispatchIndirectCommand& get() { return *this; }
	inline const VkDispatchIndirectCommand& get() const { return *this; }
};

// DisplayPropertiesKHR is a structure used only as a return type so only getters are defined
struct DisplayPropertiesKHR: private VkDisplayPropertiesKHR
{
	DisplayPropertiesKHR(){}
	DisplayPropertiesKHR(const VkDisplayPropertiesKHR& vkType): VkDisplayPropertiesKHR(vkType){}
	inline VkDisplayKHR getDisplay() const { return display; }
	inline const char* getDisplayName() const { return displayName; }
	inline const Extent2D& getPhysicalDimensions() const { return (Extent2D&)physicalDimensions; }
	inline const Extent2D& getPhysicalResolution() const { return (Extent2D&)physicalResolution; }
	inline SurfaceTransformFlagsKHR getSupportedTransforms() const { return (SurfaceTransformFlagsKHR)supportedTransforms; }
	inline VkBool32 getPlaneReorderPossible() const { return planeReorderPossible; }
	inline VkBool32 getPersistentContent() const { return persistentContent; }
	inline VkDisplayPropertiesKHR& get() { return *this; }
	inline const VkDisplayPropertiesKHR& get() const { return *this; }
};

// DisplayPlanePropertiesKHR is a structure used only as a return type so only getters are defined
struct DisplayPlanePropertiesKHR: private VkDisplayPlanePropertiesKHR
{
	DisplayPlanePropertiesKHR(){}
	DisplayPlanePropertiesKHR(const VkDisplayPlanePropertiesKHR& vkType): VkDisplayPlanePropertiesKHR(vkType){}
	inline VkDisplayKHR getCurrentDisplay() const { return currentDisplay; }
	inline uint32_t getCurrentStackIndex() const { return currentStackIndex; }
	inline VkDisplayPlanePropertiesKHR& get() { return *this; }
	inline const VkDisplayPlanePropertiesKHR& get() const { return *this; }
};

struct DisplayModeParametersKHR: private VkDisplayModeParametersKHR
{
	DisplayModeParametersKHR()
	{
		setVisibleRegion(Extent2D());
		setRefreshRate(uint32_t());
	}
	DisplayModeParametersKHR(const VkDisplayModeParametersKHR& vkType): VkDisplayModeParametersKHR(vkType){}
	DisplayModeParametersKHR(const Extent2D& visibleRegion, uint32_t refreshRate)
	{
		setVisibleRegion(visibleRegion);
		setRefreshRate(refreshRate);
	}
	inline const Extent2D& getVisibleRegion() const { return (Extent2D&)visibleRegion; }
	inline void setVisibleRegion(const Extent2D& inVisibleRegion) { memcpy(&this->visibleRegion, &inVisibleRegion, sizeof(this->visibleRegion)); }
	inline uint32_t getRefreshRate() const { return refreshRate; }
	inline void setRefreshRate(uint32_t inRefreshRate) { this->refreshRate = inRefreshRate; }
	inline VkDisplayModeParametersKHR& get() { return *this; }
	inline const VkDisplayModeParametersKHR& get() const { return *this; }
};

// DisplayModePropertiesKHR is a structure used only as a return type so only getters are defined
struct DisplayModePropertiesKHR: private VkDisplayModePropertiesKHR
{
	DisplayModePropertiesKHR(){}
	DisplayModePropertiesKHR(const VkDisplayModePropertiesKHR& vkType): VkDisplayModePropertiesKHR(vkType){}
	inline VkDisplayModeKHR getDisplayMode() const { return displayMode; }
	inline const DisplayModeParametersKHR& getParameters() const { return (DisplayModeParametersKHR&)parameters; }
	inline VkDisplayModePropertiesKHR& get() { return *this; }
	inline const VkDisplayModePropertiesKHR& get() const { return *this; }
};

// DisplayPlaneCapabilitiesKHR is a structure used only as a return type so only getters are defined
struct DisplayPlaneCapabilitiesKHR: private VkDisplayPlaneCapabilitiesKHR
{
	DisplayPlaneCapabilitiesKHR(){}
	DisplayPlaneCapabilitiesKHR(const VkDisplayPlaneCapabilitiesKHR& vkType): VkDisplayPlaneCapabilitiesKHR(vkType){}
	inline DisplayPlaneAlphaFlagsKHR getSupportedAlpha() const { return (DisplayPlaneAlphaFlagsKHR)supportedAlpha; }
	inline const Offset2D& getMinSrcPosition() const { return (Offset2D&)minSrcPosition; }
	inline const Offset2D& getMaxSrcPosition() const { return (Offset2D&)maxSrcPosition; }
	inline const Extent2D& getMinSrcExtent() const { return (Extent2D&)minSrcExtent; }
	inline const Extent2D& getMaxSrcExtent() const { return (Extent2D&)maxSrcExtent; }
	inline const Offset2D& getMinDstPosition() const { return (Offset2D&)minDstPosition; }
	inline const Offset2D& getMaxDstPosition() const { return (Offset2D&)maxDstPosition; }
	inline const Extent2D& getMinDstExtent() const { return (Extent2D&)minDstExtent; }
	inline const Extent2D& getMaxDstExtent() const { return (Extent2D&)maxDstExtent; }
	inline VkDisplayPlaneCapabilitiesKHR& get() { return *this; }
	inline const VkDisplayPlaneCapabilitiesKHR& get() const { return *this; }
};

// SurfaceCapabilitiesKHR is a structure used only as a return type so only getters are defined
struct SurfaceCapabilitiesKHR: private VkSurfaceCapabilitiesKHR
{
	SurfaceCapabilitiesKHR(){}
	SurfaceCapabilitiesKHR(const VkSurfaceCapabilitiesKHR& vkType): VkSurfaceCapabilitiesKHR(vkType){}
	inline uint32_t getMinImageCount() const { return minImageCount; }
	inline uint32_t getMaxImageCount() const { return maxImageCount; }
	inline const Extent2D& getCurrentExtent() const { return (Extent2D&)currentExtent; }
	inline const Extent2D& getMinImageExtent() const { return (Extent2D&)minImageExtent; }
	inline const Extent2D& getMaxImageExtent() const { return (Extent2D&)maxImageExtent; }
	inline uint32_t getMaxImageArrayLayers() const { return maxImageArrayLayers; }
	inline SurfaceTransformFlagsKHR getSupportedTransforms() const { return (SurfaceTransformFlagsKHR)supportedTransforms; }
	inline SurfaceTransformFlagsKHR getCurrentTransform() const { return (SurfaceTransformFlagsKHR)currentTransform; }
	inline CompositeAlphaFlagsKHR getSupportedCompositeAlpha() const { return (CompositeAlphaFlagsKHR)supportedCompositeAlpha; }
	inline ImageUsageFlags getSupportedUsageFlags() const { return (ImageUsageFlags)supportedUsageFlags; }
	inline VkSurfaceCapabilitiesKHR& get() { return *this; }
	inline const VkSurfaceCapabilitiesKHR& get() const { return *this; }
};

// SurfaceFormatKHR is a structure used only as a return type so only getters are defined
struct SurfaceFormatKHR: private VkSurfaceFormatKHR
{
	SurfaceFormatKHR(){}
	SurfaceFormatKHR(const VkSurfaceFormatKHR& vkType): VkSurfaceFormatKHR(vkType){}
	inline Format getFormat() const { return (Format)format; }
	inline ColorSpaceKHR getColorSpace() const { return (ColorSpaceKHR)colorSpace; }
	inline VkSurfaceFormatKHR& get() { return *this; }
	inline const VkSurfaceFormatKHR& get() const { return *this; }
};

// ExternalImageFormatPropertiesNV is a structure used only as a return type so only getters are defined
struct ExternalImageFormatPropertiesNV: private VkExternalImageFormatPropertiesNV
{
	ExternalImageFormatPropertiesNV(){}
	ExternalImageFormatPropertiesNV(const VkExternalImageFormatPropertiesNV& vkType): VkExternalImageFormatPropertiesNV(vkType){}
	inline const ImageFormatProperties& getImageFormatProperties() const { return (ImageFormatProperties&)imageFormatProperties; }
	inline ExternalMemoryFeatureFlagsNV getExternalMemoryFeatures() const { return (ExternalMemoryFeatureFlagsNV)externalMemoryFeatures; }
	inline ExternalMemoryHandleTypeFlagsNV getExportFromImportedHandleTypes() const { return (ExternalMemoryHandleTypeFlagsNV)exportFromImportedHandleTypes; }
	inline ExternalMemoryHandleTypeFlagsNV getCompatibleHandleTypes() const { return (ExternalMemoryHandleTypeFlagsNV)compatibleHandleTypes; }
	inline VkExternalImageFormatPropertiesNV& get() { return *this; }
	inline const VkExternalImageFormatPropertiesNV& get() const { return *this; }
};

struct IndirectCommandsTokenNVX: private VkIndirectCommandsTokenNVX
{
	IndirectCommandsTokenNVX()
	{
		setTokenType(IndirectCommandsTokenTypeNVX());
		setBuffer(VkBuffer());
		setOffset(VkDeviceSize());
	}
	IndirectCommandsTokenNVX(const VkIndirectCommandsTokenNVX& vkType): VkIndirectCommandsTokenNVX(vkType){}
	IndirectCommandsTokenNVX(IndirectCommandsTokenTypeNVX tokenType, VkBuffer buffer, VkDeviceSize offset)
	{
		setTokenType(tokenType);
		setBuffer(buffer);
		setOffset(offset);
	}
	inline IndirectCommandsTokenTypeNVX getTokenType() const { return (IndirectCommandsTokenTypeNVX)tokenType; }
	inline void setTokenType(IndirectCommandsTokenTypeNVX inTokenType) { this->tokenType = (VkIndirectCommandsTokenTypeNVX)inTokenType; }
	inline VkBuffer getBuffer() const { return buffer; }
	inline void setBuffer(VkBuffer inBuffer) { this->buffer = inBuffer; }
	inline VkDeviceSize getOffset() const { return offset; }
	inline void setOffset(VkDeviceSize inOffset) { this->offset = inOffset; }
	inline VkIndirectCommandsTokenNVX& get() { return *this; }
	inline const VkIndirectCommandsTokenNVX& get() const { return *this; }
};

struct IndirectCommandsLayoutTokenNVX: private VkIndirectCommandsLayoutTokenNVX
{
	IndirectCommandsLayoutTokenNVX()
	{
		setTokenType(IndirectCommandsTokenTypeNVX());
		setBindingUnit(uint32_t());
		setDynamicCount(uint32_t());
		setDivisor(uint32_t());
	}
	IndirectCommandsLayoutTokenNVX(const VkIndirectCommandsLayoutTokenNVX& vkType): VkIndirectCommandsLayoutTokenNVX(vkType){}
	IndirectCommandsLayoutTokenNVX(IndirectCommandsTokenTypeNVX tokenType, uint32_t bindingUnit, uint32_t dynamicCount, uint32_t divisor)
	{
		setTokenType(tokenType);
		setBindingUnit(bindingUnit);
		setDynamicCount(dynamicCount);
		setDivisor(divisor);
	}
	inline IndirectCommandsTokenTypeNVX getTokenType() const { return (IndirectCommandsTokenTypeNVX)tokenType; }
	inline void setTokenType(IndirectCommandsTokenTypeNVX inTokenType) { this->tokenType = (VkIndirectCommandsTokenTypeNVX)inTokenType; }
	inline uint32_t getBindingUnit() const { return bindingUnit; }
	inline void setBindingUnit(uint32_t inBindingUnit) { this->bindingUnit = inBindingUnit; }
	inline uint32_t getDynamicCount() const { return dynamicCount; }
	inline void setDynamicCount(uint32_t inDynamicCount) { this->dynamicCount = inDynamicCount; }
	inline uint32_t getDivisor() const { return divisor; }
	inline void setDivisor(uint32_t inDivisor) { this->divisor = inDivisor; }
	inline VkIndirectCommandsLayoutTokenNVX& get() { return *this; }
	inline const VkIndirectCommandsLayoutTokenNVX& get() const { return *this; }
};

struct ObjectTableEntryNVX: private VkObjectTableEntryNVX
{
	ObjectTableEntryNVX()
	{
		setType(ObjectEntryTypeNVX());
		setFlags(ObjectEntryUsageFlagsNVX());
	}
	ObjectTableEntryNVX(const VkObjectTableEntryNVX& vkType): VkObjectTableEntryNVX(vkType){}
	ObjectTableEntryNVX(ObjectEntryTypeNVX type, ObjectEntryUsageFlagsNVX flags)
	{
		setType(type);
		setFlags(flags);
	}
	inline ObjectEntryTypeNVX getType() const { return (ObjectEntryTypeNVX)type; }
	inline void setType(ObjectEntryTypeNVX inType) { this->type = (VkObjectEntryTypeNVX)inType; }
	inline ObjectEntryUsageFlagsNVX getFlags() const { return (ObjectEntryUsageFlagsNVX)flags; }
	inline void setFlags(ObjectEntryUsageFlagsNVX inFlags) { this->flags = (VkObjectEntryUsageFlagsNVX)inFlags; }
	inline VkObjectTableEntryNVX& get() { return *this; }
	inline const VkObjectTableEntryNVX& get() const { return *this; }
};

struct ObjectTablePipelineEntryNVX: private VkObjectTablePipelineEntryNVX
{
	ObjectTablePipelineEntryNVX()
	{
		setType(ObjectEntryTypeNVX());
		setFlags(ObjectEntryUsageFlagsNVX());
		setPipeline(VkPipeline());
	}
	ObjectTablePipelineEntryNVX(const VkObjectTablePipelineEntryNVX& vkType): VkObjectTablePipelineEntryNVX(vkType){}
	ObjectTablePipelineEntryNVX(ObjectEntryTypeNVX type, ObjectEntryUsageFlagsNVX flags, VkPipeline pipeline)
	{
		setType(type);
		setFlags(flags);
		setPipeline(pipeline);
	}
	inline ObjectEntryTypeNVX getType() const { return (ObjectEntryTypeNVX)type; }
	inline void setType(ObjectEntryTypeNVX inType) { this->type = (VkObjectEntryTypeNVX)inType; }
	inline ObjectEntryUsageFlagsNVX getFlags() const { return (ObjectEntryUsageFlagsNVX)flags; }
	inline void setFlags(ObjectEntryUsageFlagsNVX inFlags) { this->flags = (VkObjectEntryUsageFlagsNVX)inFlags; }
	inline VkPipeline getPipeline() const { return pipeline; }
	inline void setPipeline(VkPipeline inPipeline) { this->pipeline = inPipeline; }
	inline VkObjectTablePipelineEntryNVX& get() { return *this; }
	inline const VkObjectTablePipelineEntryNVX& get() const { return *this; }
};

struct ObjectTableDescriptorSetEntryNVX: private VkObjectTableDescriptorSetEntryNVX
{
	ObjectTableDescriptorSetEntryNVX()
	{
		setType(ObjectEntryTypeNVX());
		setFlags(ObjectEntryUsageFlagsNVX());
		setPipelineLayout(VkPipelineLayout());
		setDescriptorSet(VkDescriptorSet());
	}
	ObjectTableDescriptorSetEntryNVX(const VkObjectTableDescriptorSetEntryNVX& vkType): VkObjectTableDescriptorSetEntryNVX(vkType){}
	ObjectTableDescriptorSetEntryNVX(ObjectEntryTypeNVX type, ObjectEntryUsageFlagsNVX flags, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet)
	{
		setType(type);
		setFlags(flags);
		setPipelineLayout(pipelineLayout);
		setDescriptorSet(descriptorSet);
	}
	inline ObjectEntryTypeNVX getType() const { return (ObjectEntryTypeNVX)type; }
	inline void setType(ObjectEntryTypeNVX inType) { this->type = (VkObjectEntryTypeNVX)inType; }
	inline ObjectEntryUsageFlagsNVX getFlags() const { return (ObjectEntryUsageFlagsNVX)flags; }
	inline void setFlags(ObjectEntryUsageFlagsNVX inFlags) { this->flags = (VkObjectEntryUsageFlagsNVX)inFlags; }
	inline VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
	inline void setPipelineLayout(VkPipelineLayout inPipelineLayout) { this->pipelineLayout = inPipelineLayout; }
	inline VkDescriptorSet getDescriptorSet() const { return descriptorSet; }
	inline void setDescriptorSet(VkDescriptorSet inDescriptorSet) { this->descriptorSet = inDescriptorSet; }
	inline VkObjectTableDescriptorSetEntryNVX& get() { return *this; }
	inline const VkObjectTableDescriptorSetEntryNVX& get() const { return *this; }
};

struct ObjectTableVertexBufferEntryNVX: private VkObjectTableVertexBufferEntryNVX
{
	ObjectTableVertexBufferEntryNVX()
	{
		setType(ObjectEntryTypeNVX());
		setFlags(ObjectEntryUsageFlagsNVX());
		setBuffer(VkBuffer());
	}
	ObjectTableVertexBufferEntryNVX(const VkObjectTableVertexBufferEntryNVX& vkType): VkObjectTableVertexBufferEntryNVX(vkType){}
	ObjectTableVertexBufferEntryNVX(ObjectEntryTypeNVX type, ObjectEntryUsageFlagsNVX flags, VkBuffer buffer)
	{
		setType(type);
		setFlags(flags);
		setBuffer(buffer);
	}
	inline ObjectEntryTypeNVX getType() const { return (ObjectEntryTypeNVX)type; }
	inline void setType(ObjectEntryTypeNVX inType) { this->type = (VkObjectEntryTypeNVX)inType; }
	inline ObjectEntryUsageFlagsNVX getFlags() const { return (ObjectEntryUsageFlagsNVX)flags; }
	inline void setFlags(ObjectEntryUsageFlagsNVX inFlags) { this->flags = (VkObjectEntryUsageFlagsNVX)inFlags; }
	inline VkBuffer getBuffer() const { return buffer; }
	inline void setBuffer(VkBuffer inBuffer) { this->buffer = inBuffer; }
	inline VkObjectTableVertexBufferEntryNVX& get() { return *this; }
	inline const VkObjectTableVertexBufferEntryNVX& get() const { return *this; }
};

struct ObjectTableIndexBufferEntryNVX: private VkObjectTableIndexBufferEntryNVX
{
	ObjectTableIndexBufferEntryNVX()
	{
		setType(ObjectEntryTypeNVX());
		setFlags(ObjectEntryUsageFlagsNVX());
		setBuffer(VkBuffer());
		setIndexType(IndexType());
	}
	ObjectTableIndexBufferEntryNVX(const VkObjectTableIndexBufferEntryNVX& vkType): VkObjectTableIndexBufferEntryNVX(vkType){}
	ObjectTableIndexBufferEntryNVX(ObjectEntryTypeNVX type, ObjectEntryUsageFlagsNVX flags, VkBuffer buffer, IndexType indexType)
	{
		setType(type);
		setFlags(flags);
		setBuffer(buffer);
		setIndexType(indexType);
	}
	inline ObjectEntryTypeNVX getType() const { return (ObjectEntryTypeNVX)type; }
	inline void setType(ObjectEntryTypeNVX inType) { this->type = (VkObjectEntryTypeNVX)inType; }
	inline ObjectEntryUsageFlagsNVX getFlags() const { return (ObjectEntryUsageFlagsNVX)flags; }
	inline void setFlags(ObjectEntryUsageFlagsNVX inFlags) { this->flags = (VkObjectEntryUsageFlagsNVX)inFlags; }
	inline VkBuffer getBuffer() const { return buffer; }
	inline void setBuffer(VkBuffer inBuffer) { this->buffer = inBuffer; }
	inline IndexType getIndexType() const { return (IndexType)indexType; }
	inline void setIndexType(IndexType inIndexType) { this->indexType = (VkIndexType)inIndexType; }
	inline VkObjectTableIndexBufferEntryNVX& get() { return *this; }
	inline const VkObjectTableIndexBufferEntryNVX& get() const { return *this; }
};

struct ObjectTablePushConstantEntryNVX: private VkObjectTablePushConstantEntryNVX
{
	ObjectTablePushConstantEntryNVX()
	{
		setType(ObjectEntryTypeNVX());
		setFlags(ObjectEntryUsageFlagsNVX());
		setPipelineLayout(VkPipelineLayout());
		setStageFlags(ShaderStageFlags());
	}
	ObjectTablePushConstantEntryNVX(const VkObjectTablePushConstantEntryNVX& vkType): VkObjectTablePushConstantEntryNVX(vkType){}
	ObjectTablePushConstantEntryNVX(ObjectEntryTypeNVX type, ObjectEntryUsageFlagsNVX flags, VkPipelineLayout pipelineLayout, ShaderStageFlags stageFlags)
	{
		setType(type);
		setFlags(flags);
		setPipelineLayout(pipelineLayout);
		setStageFlags(stageFlags);
	}
	inline ObjectEntryTypeNVX getType() const { return (ObjectEntryTypeNVX)type; }
	inline void setType(ObjectEntryTypeNVX inType) { this->type = (VkObjectEntryTypeNVX)inType; }
	inline ObjectEntryUsageFlagsNVX getFlags() const { return (ObjectEntryUsageFlagsNVX)flags; }
	inline void setFlags(ObjectEntryUsageFlagsNVX inFlags) { this->flags = (VkObjectEntryUsageFlagsNVX)inFlags; }
	inline VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
	inline void setPipelineLayout(VkPipelineLayout inPipelineLayout) { this->pipelineLayout = inPipelineLayout; }
	inline ShaderStageFlags getStageFlags() const { return (ShaderStageFlags)stageFlags; }
	inline void setStageFlags(ShaderStageFlags inStageFlags) { this->stageFlags = (VkShaderStageFlags)inStageFlags; }
	inline VkObjectTablePushConstantEntryNVX& get() { return *this; }
	inline const VkObjectTablePushConstantEntryNVX& get() const { return *this; }
};

struct ConformanceVersion: private VkConformanceVersion
{
	ConformanceVersion()
	{
		setMajor(uint8_t());
		setMinor(uint8_t());
		setSubminor(uint8_t());
		setPatch(uint8_t());
	}
	ConformanceVersion(const VkConformanceVersion& vkType): VkConformanceVersion(vkType){}
	ConformanceVersion(uint8_t major, uint8_t minor, uint8_t subminor, uint8_t patch)
	{
		setMajor(major);
		setMinor(minor);
		setSubminor(subminor);
		setPatch(patch);
	}
	inline uint8_t getMajor() const { return major; }
	inline void setMajor(uint8_t inMajor) { this->major = inMajor; }
	inline uint8_t getMinor() const { return minor; }
	inline void setMinor(uint8_t inMinor) { this->minor = inMinor; }
	inline uint8_t getSubminor() const { return subminor; }
	inline void setSubminor(uint8_t inSubminor) { this->subminor = inSubminor; }
	inline uint8_t getPatch() const { return patch; }
	inline void setPatch(uint8_t inPatch) { this->patch = inPatch; }
	inline VkConformanceVersion& get() { return *this; }
	inline const VkConformanceVersion& get() const { return *this; }
};

struct RectLayerKHR: private VkRectLayerKHR
{
	RectLayerKHR()
	{
		setOffset(Offset2D());
		setExtent(Extent2D());
		setLayer(uint32_t());
	}
	RectLayerKHR(const VkRectLayerKHR& vkType): VkRectLayerKHR(vkType){}
	RectLayerKHR(const Offset2D& offset, const Extent2D& extent, uint32_t layer)
	{
		setOffset(offset);
		setExtent(extent);
		setLayer(layer);
	}
	inline const Offset2D& getOffset() const { return (Offset2D&)offset; }
	inline void setOffset(const Offset2D& inOffset) { memcpy(&this->offset, &inOffset, sizeof(this->offset)); }
	inline const Extent2D& getExtent() const { return (Extent2D&)extent; }
	inline void setExtent(const Extent2D& inExtent) { memcpy(&this->extent, &inExtent, sizeof(this->extent)); }
	inline uint32_t getLayer() const { return layer; }
	inline void setLayer(uint32_t inLayer) { this->layer = inLayer; }
	inline VkRectLayerKHR& get() { return *this; }
	inline const VkRectLayerKHR& get() const { return *this; }
};

struct PresentRegionKHR: private VkPresentRegionKHR
{
	PresentRegionKHR()
	{
		setRectangleCount(uint32_t());
		setPRectangles(nullptr);
	}
	PresentRegionKHR(const VkPresentRegionKHR& vkType): VkPresentRegionKHR(vkType){}
	PresentRegionKHR(uint32_t rectangleCount, RectLayerKHR* pRectangles)
	{
		setRectangleCount(rectangleCount);
		setPRectangles(pRectangles);
	}
	inline uint32_t getRectangleCount() const { return rectangleCount; }
	inline void setRectangleCount(uint32_t inRectangleCount) { this->rectangleCount = inRectangleCount; }
	inline const RectLayerKHR* getPRectangles() const { return (RectLayerKHR*)pRectangles; }
	inline void setPRectangles(RectLayerKHR* inPRectangles) { this->pRectangles = (VkRectLayerKHR*)inPRectangles; }
	inline VkPresentRegionKHR& get() { return *this; }
	inline const VkPresentRegionKHR& get() const { return *this; }
};

// ExternalMemoryProperties is a structure used only as a return type so only getters are defined
struct ExternalMemoryProperties: private VkExternalMemoryProperties
{
	ExternalMemoryProperties(){}
	ExternalMemoryProperties(const VkExternalMemoryProperties& vkType): VkExternalMemoryProperties(vkType){}
	inline ExternalMemoryFeatureFlags getExternalMemoryFeatures() const { return (ExternalMemoryFeatureFlags)externalMemoryFeatures; }
	inline ExternalMemoryHandleTypeFlags getExportFromImportedHandleTypes() const { return (ExternalMemoryHandleTypeFlags)exportFromImportedHandleTypes; }
	inline ExternalMemoryHandleTypeFlags getCompatibleHandleTypes() const { return (ExternalMemoryHandleTypeFlags)compatibleHandleTypes; }
	inline VkExternalMemoryProperties& get() { return *this; }
	inline const VkExternalMemoryProperties& get() const { return *this; }
};

struct DescriptorUpdateTemplateEntry: private VkDescriptorUpdateTemplateEntry
{
	DescriptorUpdateTemplateEntry()
	{
		setDstBinding(uint32_t());
		setDstArrayElement(uint32_t());
		setDescriptorCount(uint32_t());
		setDescriptorType(DescriptorType());
		setOffset(size_t());
		setStride(size_t());
	}
	DescriptorUpdateTemplateEntry(const VkDescriptorUpdateTemplateEntry& vkType): VkDescriptorUpdateTemplateEntry(vkType){}
	DescriptorUpdateTemplateEntry(uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, DescriptorType descriptorType, size_t offset, size_t stride)
	{
		setDstBinding(dstBinding);
		setDstArrayElement(dstArrayElement);
		setDescriptorCount(descriptorCount);
		setDescriptorType(descriptorType);
		setOffset(offset);
		setStride(stride);
	}
	inline uint32_t getDstBinding() const { return dstBinding; }
	inline void setDstBinding(uint32_t inDstBinding) { this->dstBinding = inDstBinding; }
	inline uint32_t getDstArrayElement() const { return dstArrayElement; }
	inline void setDstArrayElement(uint32_t inDstArrayElement) { this->dstArrayElement = inDstArrayElement; }
	inline uint32_t getDescriptorCount() const { return descriptorCount; }
	inline void setDescriptorCount(uint32_t inDescriptorCount) { this->descriptorCount = inDescriptorCount; }
	inline DescriptorType getDescriptorType() const { return (DescriptorType)descriptorType; }
	inline void setDescriptorType(DescriptorType inDescriptorType) { this->descriptorType = (VkDescriptorType)inDescriptorType; }
	inline size_t getOffset() const { return offset; }
	inline void setOffset(size_t inOffset) { this->offset = inOffset; }
	inline size_t getStride() const { return stride; }
	inline void setStride(size_t inStride) { this->stride = inStride; }
	inline VkDescriptorUpdateTemplateEntry& get() { return *this; }
	inline const VkDescriptorUpdateTemplateEntry& get() const { return *this; }
};

struct XYColorEXT: private VkXYColorEXT
{
	XYColorEXT()
	{
		setX(float());
		setY(float());
	}
	XYColorEXT(const VkXYColorEXT& vkType): VkXYColorEXT(vkType){}
	XYColorEXT(float x, float y)
	{
		setX(x);
		setY(y);
	}
	inline float getX() const { return x; }
	inline void setX(float inX) { this->x = inX; }
	inline float getY() const { return y; }
	inline void setY(float inY) { this->y = inY; }
	inline VkXYColorEXT& get() { return *this; }
	inline const VkXYColorEXT& get() const { return *this; }
};

// RefreshCycleDurationGOOGLE is a structure used only as a return type so only getters are defined
struct RefreshCycleDurationGOOGLE: private VkRefreshCycleDurationGOOGLE
{
	RefreshCycleDurationGOOGLE(){}
	RefreshCycleDurationGOOGLE(const VkRefreshCycleDurationGOOGLE& vkType): VkRefreshCycleDurationGOOGLE(vkType){}
	inline uint64_t getRefreshDuration() const { return refreshDuration; }
	inline VkRefreshCycleDurationGOOGLE& get() { return *this; }
	inline const VkRefreshCycleDurationGOOGLE& get() const { return *this; }
};

// PastPresentationTimingGOOGLE is a structure used only as a return type so only getters are defined
struct PastPresentationTimingGOOGLE: private VkPastPresentationTimingGOOGLE
{
	PastPresentationTimingGOOGLE(){}
	PastPresentationTimingGOOGLE(const VkPastPresentationTimingGOOGLE& vkType): VkPastPresentationTimingGOOGLE(vkType){}
	inline uint32_t getPresentID() const { return presentID; }
	inline uint64_t getDesiredPresentTime() const { return desiredPresentTime; }
	inline uint64_t getActualPresentTime() const { return actualPresentTime; }
	inline uint64_t getEarliestPresentTime() const { return earliestPresentTime; }
	inline uint64_t getPresentMargin() const { return presentMargin; }
	inline VkPastPresentationTimingGOOGLE& get() { return *this; }
	inline const VkPastPresentationTimingGOOGLE& get() const { return *this; }
};

struct PresentTimeGOOGLE: private VkPresentTimeGOOGLE
{
	PresentTimeGOOGLE()
	{
		setPresentID(uint32_t());
		setDesiredPresentTime(uint64_t());
	}
	PresentTimeGOOGLE(const VkPresentTimeGOOGLE& vkType): VkPresentTimeGOOGLE(vkType){}
	PresentTimeGOOGLE(uint32_t presentID, uint64_t desiredPresentTime)
	{
		setPresentID(presentID);
		setDesiredPresentTime(desiredPresentTime);
	}
	inline uint32_t getPresentID() const { return presentID; }
	inline void setPresentID(uint32_t inPresentID) { this->presentID = inPresentID; }
	inline uint64_t getDesiredPresentTime() const { return desiredPresentTime; }
	inline void setDesiredPresentTime(uint64_t inDesiredPresentTime) { this->desiredPresentTime = inDesiredPresentTime; }
	inline VkPresentTimeGOOGLE& get() { return *this; }
	inline const VkPresentTimeGOOGLE& get() const { return *this; }
};

struct ViewportWScalingNV: private VkViewportWScalingNV
{
	ViewportWScalingNV()
	{
		setXcoeff(float());
		setYcoeff(float());
	}
	ViewportWScalingNV(const VkViewportWScalingNV& vkType): VkViewportWScalingNV(vkType){}
	ViewportWScalingNV(float xcoeff, float ycoeff)
	{
		setXcoeff(xcoeff);
		setYcoeff(ycoeff);
	}
	inline float getXcoeff() const { return xcoeff; }
	inline void setXcoeff(float inXcoeff) { this->xcoeff = inXcoeff; }
	inline float getYcoeff() const { return ycoeff; }
	inline void setYcoeff(float inYcoeff) { this->ycoeff = inYcoeff; }
	inline VkViewportWScalingNV& get() { return *this; }
	inline const VkViewportWScalingNV& get() const { return *this; }
};

struct ViewportSwizzleNV: private VkViewportSwizzleNV
{
	ViewportSwizzleNV()
	{
		setX(ViewportCoordinateSwizzleNV());
		setY(ViewportCoordinateSwizzleNV());
		setZ(ViewportCoordinateSwizzleNV());
		setW(ViewportCoordinateSwizzleNV());
	}
	ViewportSwizzleNV(const VkViewportSwizzleNV& vkType): VkViewportSwizzleNV(vkType){}
	ViewportSwizzleNV(ViewportCoordinateSwizzleNV x, ViewportCoordinateSwizzleNV y, ViewportCoordinateSwizzleNV z, ViewportCoordinateSwizzleNV w)
	{
		setX(x);
		setY(y);
		setZ(z);
		setW(w);
	}
	inline ViewportCoordinateSwizzleNV getX() const { return (ViewportCoordinateSwizzleNV)x; }
	inline void setX(ViewportCoordinateSwizzleNV inX) { this->x = (VkViewportCoordinateSwizzleNV)inX; }
	inline ViewportCoordinateSwizzleNV getY() const { return (ViewportCoordinateSwizzleNV)y; }
	inline void setY(ViewportCoordinateSwizzleNV inY) { this->y = (VkViewportCoordinateSwizzleNV)inY; }
	inline ViewportCoordinateSwizzleNV getZ() const { return (ViewportCoordinateSwizzleNV)z; }
	inline void setZ(ViewportCoordinateSwizzleNV inZ) { this->z = (VkViewportCoordinateSwizzleNV)inZ; }
	inline ViewportCoordinateSwizzleNV getW() const { return (ViewportCoordinateSwizzleNV)w; }
	inline void setW(ViewportCoordinateSwizzleNV inW) { this->w = (VkViewportCoordinateSwizzleNV)inW; }
	inline VkViewportSwizzleNV& get() { return *this; }
	inline const VkViewportSwizzleNV& get() const { return *this; }
};

struct InputAttachmentAspectReference: private VkInputAttachmentAspectReference
{
	InputAttachmentAspectReference()
	{
		setSubpass(uint32_t());
		setInputAttachmentIndex(uint32_t());
		setAspectMask(ImageAspectFlags());
	}
	InputAttachmentAspectReference(const VkInputAttachmentAspectReference& vkType): VkInputAttachmentAspectReference(vkType){}
	InputAttachmentAspectReference(uint32_t subpass, uint32_t inputAttachmentIndex, ImageAspectFlags aspectMask)
	{
		setSubpass(subpass);
		setInputAttachmentIndex(inputAttachmentIndex);
		setAspectMask(aspectMask);
	}
	inline uint32_t getSubpass() const { return subpass; }
	inline void setSubpass(uint32_t inSubpass) { this->subpass = inSubpass; }
	inline uint32_t getInputAttachmentIndex() const { return inputAttachmentIndex; }
	inline void setInputAttachmentIndex(uint32_t inInputAttachmentIndex) { this->inputAttachmentIndex = inInputAttachmentIndex; }
	inline ImageAspectFlags getAspectMask() const { return (ImageAspectFlags)aspectMask; }
	inline void setAspectMask(ImageAspectFlags inAspectMask) { this->aspectMask = (VkImageAspectFlags)inAspectMask; }
	inline VkInputAttachmentAspectReference& get() { return *this; }
	inline const VkInputAttachmentAspectReference& get() const { return *this; }
};

struct SampleLocationEXT: private VkSampleLocationEXT
{
	SampleLocationEXT()
	{
		setX(float());
		setY(float());
	}
	SampleLocationEXT(const VkSampleLocationEXT& vkType): VkSampleLocationEXT(vkType){}
	SampleLocationEXT(float x, float y)
	{
		setX(x);
		setY(y);
	}
	inline float getX() const { return x; }
	inline void setX(float inX) { this->x = inX; }
	inline float getY() const { return y; }
	inline void setY(float inY) { this->y = inY; }
	inline VkSampleLocationEXT& get() { return *this; }
	inline const VkSampleLocationEXT& get() const { return *this; }
};

struct AttachmentSampleLocationsEXT: private VkAttachmentSampleLocationsEXT
{
	AttachmentSampleLocationsEXT()
	{
		setAttachmentIndex(uint32_t());
		setSampleLocationsInfo(VkSampleLocationsInfoEXT());
	}
	AttachmentSampleLocationsEXT(const VkAttachmentSampleLocationsEXT& vkType): VkAttachmentSampleLocationsEXT(vkType){}
	AttachmentSampleLocationsEXT(uint32_t attachmentIndex, const VkSampleLocationsInfoEXT& sampleLocationsInfo)
	{
		setAttachmentIndex(attachmentIndex);
		setSampleLocationsInfo(sampleLocationsInfo);
	}
	inline uint32_t getAttachmentIndex() const { return attachmentIndex; }
	inline void setAttachmentIndex(uint32_t inAttachmentIndex) { this->attachmentIndex = inAttachmentIndex; }
	inline const VkSampleLocationsInfoEXT& getSampleLocationsInfo() const { return sampleLocationsInfo; }
	inline void setSampleLocationsInfo(const VkSampleLocationsInfoEXT& inSampleLocationsInfo) { memcpy(&this->sampleLocationsInfo, &inSampleLocationsInfo, sizeof(this->sampleLocationsInfo)); }
	inline VkAttachmentSampleLocationsEXT& get() { return *this; }
	inline const VkAttachmentSampleLocationsEXT& get() const { return *this; }
};

struct SubpassSampleLocationsEXT: private VkSubpassSampleLocationsEXT
{
	SubpassSampleLocationsEXT()
	{
		setSubpassIndex(uint32_t());
		setSampleLocationsInfo(VkSampleLocationsInfoEXT());
	}
	SubpassSampleLocationsEXT(const VkSubpassSampleLocationsEXT& vkType): VkSubpassSampleLocationsEXT(vkType){}
	SubpassSampleLocationsEXT(uint32_t subpassIndex, const VkSampleLocationsInfoEXT& sampleLocationsInfo)
	{
		setSubpassIndex(subpassIndex);
		setSampleLocationsInfo(sampleLocationsInfo);
	}
	inline uint32_t getSubpassIndex() const { return subpassIndex; }
	inline void setSubpassIndex(uint32_t inSubpassIndex) { this->subpassIndex = inSubpassIndex; }
	inline const VkSampleLocationsInfoEXT& getSampleLocationsInfo() const { return sampleLocationsInfo; }
	inline void setSampleLocationsInfo(const VkSampleLocationsInfoEXT& inSampleLocationsInfo) { memcpy(&this->sampleLocationsInfo, &inSampleLocationsInfo, sizeof(this->sampleLocationsInfo)); }
	inline VkSubpassSampleLocationsEXT& get() { return *this; }
	inline const VkSubpassSampleLocationsEXT& get() const { return *this; }
};

// ShaderResourceUsageAMD is a structure used only as a return type so only getters are defined
struct ShaderResourceUsageAMD: private VkShaderResourceUsageAMD
{
	ShaderResourceUsageAMD(){}
	ShaderResourceUsageAMD(const VkShaderResourceUsageAMD& vkType): VkShaderResourceUsageAMD(vkType){}
	inline uint32_t getNumUsedVgprs() const { return numUsedVgprs; }
	inline uint32_t getNumUsedSgprs() const { return numUsedSgprs; }
	inline uint32_t getLdsSizePerLocalWorkGroup() const { return ldsSizePerLocalWorkGroup; }
	inline size_t getLdsUsageSizeInBytes() const { return ldsUsageSizeInBytes; }
	inline size_t getScratchMemUsageInBytes() const { return scratchMemUsageInBytes; }
	inline VkShaderResourceUsageAMD& get() { return *this; }
	inline const VkShaderResourceUsageAMD& get() const { return *this; }
};

// ShaderStatisticsInfoAMD is a structure used only as a return type so only getters are defined
struct ShaderStatisticsInfoAMD: private VkShaderStatisticsInfoAMD
{
	ShaderStatisticsInfoAMD(){}
	ShaderStatisticsInfoAMD(const VkShaderStatisticsInfoAMD& vkType): VkShaderStatisticsInfoAMD(vkType){}
	inline ShaderStageFlags getShaderStageMask() const { return (ShaderStageFlags)shaderStageMask; }
	inline const ShaderResourceUsageAMD& getResourceUsage() const { return (ShaderResourceUsageAMD&)resourceUsage; }
	inline uint32_t getNumPhysicalVgprs() const { return numPhysicalVgprs; }
	inline uint32_t getNumPhysicalSgprs() const { return numPhysicalSgprs; }
	inline uint32_t getNumAvailableVgprs() const { return numAvailableVgprs; }
	inline uint32_t getNumAvailableSgprs() const { return numAvailableSgprs; }
	inline const uint32_t* getComputeWorkGroupSize() const { return computeWorkGroupSize; }
	inline VkShaderStatisticsInfoAMD& get() { return *this; }
	inline const VkShaderStatisticsInfoAMD& get() const { return *this; }
};

struct VertexInputBindingDivisorDescriptionEXT: private VkVertexInputBindingDivisorDescriptionEXT
{
	VertexInputBindingDivisorDescriptionEXT()
	{
		setBinding(uint32_t());
		setDivisor(uint32_t());
	}
	VertexInputBindingDivisorDescriptionEXT(const VkVertexInputBindingDivisorDescriptionEXT& vkType): VkVertexInputBindingDivisorDescriptionEXT(vkType){}
	VertexInputBindingDivisorDescriptionEXT(uint32_t binding, uint32_t divisor)
	{
		setBinding(binding);
		setDivisor(divisor);
	}
	inline uint32_t getBinding() const { return binding; }
	inline void setBinding(uint32_t inBinding) { this->binding = inBinding; }
	inline uint32_t getDivisor() const { return divisor; }
	inline void setDivisor(uint32_t inDivisor) { this->divisor = inDivisor; }
	inline VkVertexInputBindingDivisorDescriptionEXT& get() { return *this; }
	inline const VkVertexInputBindingDivisorDescriptionEXT& get() const { return *this; }
};

struct ShadingRatePaletteNV: private VkShadingRatePaletteNV
{
	ShadingRatePaletteNV()
	{
		setShadingRatePaletteEntryCount(uint32_t());
		setPShadingRatePaletteEntries(nullptr);
	}
	ShadingRatePaletteNV(const VkShadingRatePaletteNV& vkType): VkShadingRatePaletteNV(vkType){}
	ShadingRatePaletteNV(uint32_t shadingRatePaletteEntryCount, ShadingRatePaletteEntryNV* pShadingRatePaletteEntries)
	{
		setShadingRatePaletteEntryCount(shadingRatePaletteEntryCount);
		setPShadingRatePaletteEntries(pShadingRatePaletteEntries);
	}
	inline uint32_t getShadingRatePaletteEntryCount() const { return shadingRatePaletteEntryCount; }
	inline void setShadingRatePaletteEntryCount(uint32_t inShadingRatePaletteEntryCount) { this->shadingRatePaletteEntryCount = inShadingRatePaletteEntryCount; }
	inline const ShadingRatePaletteEntryNV* getPShadingRatePaletteEntries() const { return (ShadingRatePaletteEntryNV*)pShadingRatePaletteEntries; }
	inline void setPShadingRatePaletteEntries(ShadingRatePaletteEntryNV* inPShadingRatePaletteEntries) { this->pShadingRatePaletteEntries = (VkShadingRatePaletteEntryNV*)inPShadingRatePaletteEntries; }
	inline VkShadingRatePaletteNV& get() { return *this; }
	inline const VkShadingRatePaletteNV& get() const { return *this; }
};

struct CoarseSampleLocationNV: private VkCoarseSampleLocationNV
{
	CoarseSampleLocationNV()
	{
		setPixelX(uint32_t());
		setPixelY(uint32_t());
		setSample(uint32_t());
	}
	CoarseSampleLocationNV(const VkCoarseSampleLocationNV& vkType): VkCoarseSampleLocationNV(vkType){}
	CoarseSampleLocationNV(uint32_t pixelX, uint32_t pixelY, uint32_t sample)
	{
		setPixelX(pixelX);
		setPixelY(pixelY);
		setSample(sample);
	}
	inline uint32_t getPixelX() const { return pixelX; }
	inline void setPixelX(uint32_t inPixelX) { this->pixelX = inPixelX; }
	inline uint32_t getPixelY() const { return pixelY; }
	inline void setPixelY(uint32_t inPixelY) { this->pixelY = inPixelY; }
	inline uint32_t getSample() const { return sample; }
	inline void setSample(uint32_t inSample) { this->sample = inSample; }
	inline VkCoarseSampleLocationNV& get() { return *this; }
	inline const VkCoarseSampleLocationNV& get() const { return *this; }
};

struct CoarseSampleOrderCustomNV: private VkCoarseSampleOrderCustomNV
{
	CoarseSampleOrderCustomNV()
	{
		setShadingRate(ShadingRatePaletteEntryNV());
		setSampleCount(uint32_t());
		setSampleLocationCount(uint32_t());
		setPSampleLocations(nullptr);
	}
	CoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV& vkType): VkCoarseSampleOrderCustomNV(vkType){}
	CoarseSampleOrderCustomNV(ShadingRatePaletteEntryNV shadingRate, uint32_t sampleCount, uint32_t sampleLocationCount, CoarseSampleLocationNV* pSampleLocations)
	{
		setShadingRate(shadingRate);
		setSampleCount(sampleCount);
		setSampleLocationCount(sampleLocationCount);
		setPSampleLocations(pSampleLocations);
	}
	inline ShadingRatePaletteEntryNV getShadingRate() const { return (ShadingRatePaletteEntryNV)shadingRate; }
	inline void setShadingRate(ShadingRatePaletteEntryNV inShadingRate) { this->shadingRate = (VkShadingRatePaletteEntryNV)inShadingRate; }
	inline uint32_t getSampleCount() const { return sampleCount; }
	inline void setSampleCount(uint32_t inSampleCount) { this->sampleCount = inSampleCount; }
	inline uint32_t getSampleLocationCount() const { return sampleLocationCount; }
	inline void setSampleLocationCount(uint32_t inSampleLocationCount) { this->sampleLocationCount = inSampleLocationCount; }
	inline const CoarseSampleLocationNV* getPSampleLocations() const { return (CoarseSampleLocationNV*)pSampleLocations; }
	inline void setPSampleLocations(CoarseSampleLocationNV* inPSampleLocations) { this->pSampleLocations = (VkCoarseSampleLocationNV*)inPSampleLocations; }
	inline VkCoarseSampleOrderCustomNV& get() { return *this; }
	inline const VkCoarseSampleOrderCustomNV& get() const { return *this; }
};

struct DrawMeshTasksIndirectCommandNV: private VkDrawMeshTasksIndirectCommandNV
{
	DrawMeshTasksIndirectCommandNV()
	{
		setTaskCount(uint32_t());
		setFirstTask(uint32_t());
	}
	DrawMeshTasksIndirectCommandNV(const VkDrawMeshTasksIndirectCommandNV& vkType): VkDrawMeshTasksIndirectCommandNV(vkType){}
	DrawMeshTasksIndirectCommandNV(uint32_t taskCount, uint32_t firstTask)
	{
		setTaskCount(taskCount);
		setFirstTask(firstTask);
	}
	inline uint32_t getTaskCount() const { return taskCount; }
	inline void setTaskCount(uint32_t inTaskCount) { this->taskCount = inTaskCount; }
	inline uint32_t getFirstTask() const { return firstTask; }
	inline void setFirstTask(uint32_t inFirstTask) { this->firstTask = inFirstTask; }
	inline VkDrawMeshTasksIndirectCommandNV& get() { return *this; }
	inline const VkDrawMeshTasksIndirectCommandNV& get() const { return *this; }
};

struct GeometryDataNV: private VkGeometryDataNV
{
	GeometryDataNV()
	{
		setTriangles(VkGeometryTrianglesNV());
		setAabbs(VkGeometryAABBNV());
	}
	GeometryDataNV(const VkGeometryDataNV& vkType): VkGeometryDataNV(vkType){}
	GeometryDataNV(const VkGeometryTrianglesNV& triangles, const VkGeometryAABBNV& aabbs)
	{
		setTriangles(triangles);
		setAabbs(aabbs);
	}
	inline const VkGeometryTrianglesNV& getTriangles() const { return triangles; }
	inline void setTriangles(const VkGeometryTrianglesNV& inTriangles) { memcpy(&this->triangles, &inTriangles, sizeof(this->triangles)); }
	inline const VkGeometryAABBNV& getAabbs() const { return aabbs; }
	inline void setAabbs(const VkGeometryAABBNV& inAabbs) { memcpy(&this->aabbs, &inAabbs, sizeof(this->aabbs)); }
	inline VkGeometryDataNV& get() { return *this; }
	inline const VkGeometryDataNV& get() const { return *this; }
};

// DrmFormatModifierPropertiesEXT is a structure used only as a return type so only getters are defined
struct DrmFormatModifierPropertiesEXT: private VkDrmFormatModifierPropertiesEXT
{
	DrmFormatModifierPropertiesEXT(){}
	DrmFormatModifierPropertiesEXT(const VkDrmFormatModifierPropertiesEXT& vkType): VkDrmFormatModifierPropertiesEXT(vkType){}
	inline uint64_t getDrmFormatModifier() const { return drmFormatModifier; }
	inline uint32_t getDrmFormatModifierPlaneCount() const { return drmFormatModifierPlaneCount; }
	inline FormatFeatureFlags getDrmFormatModifierTilingFeatures() const { return (FormatFeatureFlags)drmFormatModifierTilingFeatures; }
	inline VkDrmFormatModifierPropertiesEXT& get() { return *this; }
	inline const VkDrmFormatModifierPropertiesEXT& get() const { return *this; }
};

// PipelineCreationFeedbackEXT is a structure used only as a return type so only getters are defined
struct PipelineCreationFeedbackEXT: private VkPipelineCreationFeedbackEXT
{
	PipelineCreationFeedbackEXT(){}
	PipelineCreationFeedbackEXT(const VkPipelineCreationFeedbackEXT& vkType): VkPipelineCreationFeedbackEXT(vkType){}
	inline PipelineCreationFeedbackFlagsEXT getFlags() const { return (PipelineCreationFeedbackFlagsEXT)flags; }
	inline uint64_t getDuration() const { return duration; }
	inline VkPipelineCreationFeedbackEXT& get() { return *this; }
	inline const VkPipelineCreationFeedbackEXT& get() const { return *this; }
};

struct PerformanceCounterResultKHR
{
private:
	VkPerformanceCounterResultKHR _PerformanceCounterResultKHR;
public:
	PerformanceCounterResultKHR()
	{
		setInt32(int32_t());
		setInt64(int64_t());
		setUint32(uint32_t());
		setUint64(uint64_t());
		setFloat32(float());
		setFloat64(double());
	}
	PerformanceCounterResultKHR(const VkPerformanceCounterResultKHR& vkType): _PerformanceCounterResultKHR(vkType){}
	PerformanceCounterResultKHR(int32_t int32, int64_t int64, uint32_t uint32, uint64_t uint64, float float32, double float64)
	{
		setInt32(int32);
		setInt64(int64);
		setUint32(uint32);
		setUint64(uint64);
		setFloat32(float32);
		setFloat64(float64);
	}
	inline int32_t getInt32() const { return _PerformanceCounterResultKHR.int32; }
	inline void setInt32(int32_t inInt32) { this->_PerformanceCounterResultKHR.int32 = inInt32; }
	inline int64_t getInt64() const { return _PerformanceCounterResultKHR.int64; }
	inline void setInt64(int64_t inInt64) { this->_PerformanceCounterResultKHR.int64 = inInt64; }
	inline uint32_t getUint32() const { return _PerformanceCounterResultKHR.uint32; }
	inline void setUint32(uint32_t inUint32) { this->_PerformanceCounterResultKHR.uint32 = inUint32; }
	inline uint64_t getUint64() const { return _PerformanceCounterResultKHR.uint64; }
	inline void setUint64(uint64_t inUint64) { this->_PerformanceCounterResultKHR.uint64 = inUint64; }
	inline float getFloat32() const { return _PerformanceCounterResultKHR.float32; }
	inline void setFloat32(float inFloat32) { this->_PerformanceCounterResultKHR.float32 = inFloat32; }
	inline double getFloat64() const { return _PerformanceCounterResultKHR.float64; }
	inline void setFloat64(double inFloat64) { this->_PerformanceCounterResultKHR.float64 = inFloat64; }
	inline VkPerformanceCounterResultKHR& get() { return _PerformanceCounterResultKHR; }
	inline const VkPerformanceCounterResultKHR& get() const { return _PerformanceCounterResultKHR; }
};

struct PerformanceValueDataINTEL
{
private:
	VkPerformanceValueDataINTEL _PerformanceValueDataINTEL;
public:
	PerformanceValueDataINTEL()
	{
		setValue32(uint32_t());
		setValue64(uint64_t());
		setValueFloat(float());
		setValueBool(VkBool32());
		setValueString(nullptr);
	}
	PerformanceValueDataINTEL(const VkPerformanceValueDataINTEL& vkType): _PerformanceValueDataINTEL(vkType){}
	PerformanceValueDataINTEL(uint32_t value32, uint64_t value64, float valueFloat, VkBool32 valueBool, char* valueString)
	{
		setValue32(value32);
		setValue64(value64);
		setValueFloat(valueFloat);
		setValueBool(valueBool);
		setValueString(valueString);
	}
	inline uint32_t getValue32() const { return _PerformanceValueDataINTEL.value32; }
	inline void setValue32(uint32_t inValue32) { this->_PerformanceValueDataINTEL.value32 = inValue32; }
	inline uint64_t getValue64() const { return _PerformanceValueDataINTEL.value64; }
	inline void setValue64(uint64_t inValue64) { this->_PerformanceValueDataINTEL.value64 = inValue64; }
	inline float getValueFloat() const { return _PerformanceValueDataINTEL.valueFloat; }
	inline void setValueFloat(float inValueFloat) { this->_PerformanceValueDataINTEL.valueFloat = inValueFloat; }
	inline VkBool32 getValueBool() const { return _PerformanceValueDataINTEL.valueBool; }
	inline void setValueBool(VkBool32 inValueBool) { this->_PerformanceValueDataINTEL.valueBool = inValueBool; }
	inline const char* getValueString() const { return _PerformanceValueDataINTEL.valueString; }
	inline void setValueString(char* inValueString) { this->_PerformanceValueDataINTEL.valueString = inValueString; }
	inline VkPerformanceValueDataINTEL& get() { return _PerformanceValueDataINTEL; }
	inline const VkPerformanceValueDataINTEL& get() const { return _PerformanceValueDataINTEL; }
};

struct PerformanceValueINTEL: private VkPerformanceValueINTEL
{
	PerformanceValueINTEL()
	{
		setType(PerformanceValueTypeINTEL());
		setData(PerformanceValueDataINTEL());
	}
	PerformanceValueINTEL(const VkPerformanceValueINTEL& vkType): VkPerformanceValueINTEL(vkType){}
	PerformanceValueINTEL(PerformanceValueTypeINTEL type, const PerformanceValueDataINTEL& data)
	{
		setType(type);
		setData(data);
	}
	inline PerformanceValueTypeINTEL getType() const { return (PerformanceValueTypeINTEL)type; }
	inline void setType(PerformanceValueTypeINTEL inType) { this->type = (VkPerformanceValueTypeINTEL)inType; }
	inline const PerformanceValueDataINTEL& getData() const { return (PerformanceValueDataINTEL&)data; }
	inline void setData(const PerformanceValueDataINTEL& inData) { memcpy(&this->data, &inData, sizeof(this->data)); }
	inline VkPerformanceValueINTEL& get() { return *this; }
	inline const VkPerformanceValueINTEL& get() const { return *this; }
};

// PipelineExecutableStatisticValueKHR is a structure used only as a return type so only getters are defined
struct PipelineExecutableStatisticValueKHR
{
private:
	VkPipelineExecutableStatisticValueKHR _PipelineExecutableStatisticValueKHR;
public:
	PipelineExecutableStatisticValueKHR(){}
	PipelineExecutableStatisticValueKHR(const VkPipelineExecutableStatisticValueKHR& vkType): _PipelineExecutableStatisticValueKHR(vkType){}
	inline VkBool32 getB32() const { return _PipelineExecutableStatisticValueKHR.b32; }
	inline int64_t getI64() const { return _PipelineExecutableStatisticValueKHR.i64; }
	inline uint64_t getU64() const { return _PipelineExecutableStatisticValueKHR.u64; }
	inline double getF64() const { return _PipelineExecutableStatisticValueKHR.f64; }
	inline VkPipelineExecutableStatisticValueKHR& get() { return _PipelineExecutableStatisticValueKHR; }
	inline const VkPipelineExecutableStatisticValueKHR& get() const { return _PipelineExecutableStatisticValueKHR; }
};


// PVRVk Errors

namespace impl {

	/// <summary>Checks whether a debugger can be found for the current running process (on Windows and Linux only).
	/// The prescene of a debugger can be used to provide additional helpful functionality for debugging application issues one of which could be to break in the
	/// debugger when an exception is thrown. Being able to have the debugger break on such a thrown exception provides by far the most seamless and constructive environment for
	/// fixing an issue causing the exception to be thrown due to the full state and stack trace being present at the point in which the issue has occurred rather
	/// than relying on error logic handling.</summary>
	/// <returns>True if a debugger can be found for the current running process else False.</returns>
	inline static bool isDebuggerPresent()
	{
		// only check once for whether the debugger is present as this may not be efficient to determine
		static bool isUsingDebugger = false;
		static bool haveCheckedForDebugger = false;
		if (!haveCheckedForDebugger)
		{
#if defined(_MSC_VER)
			if (IsDebuggerPresent())
			{
				isUsingDebugger = true;
			}
#elif defined(__linux__)
			// reference implementation taken from: https://stackoverflow.com/a/24969863
			char buf[1024];

			int status_fd = open("/proc/self/status", O_RDONLY);
			if (status_fd == -1)
			{
				isUsingDebugger = false;
			}
			else
			{
				ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);
				if (num_read > 0)
				{
					static const char TracerPid[] = "TracerPid:";
					char* tracer_pid;

					buf[num_read] = 0;
					tracer_pid = strstr(buf, TracerPid);
					if (tracer_pid)
					{
						isUsingDebugger = !!atoi(tracer_pid + sizeof(TracerPid) - 1);
					}
				}
			}
#endif
			haveCheckedForDebugger = true;
		}

		return isUsingDebugger;
	}

	/// <summary>If supported on the platform, makes the debugger break at this line. Used for Assertions on Visual Studio</summary>
	inline void debuggerBreak()
	{
		if (isDebuggerPresent())
		{
#if defined(__linux__)
			{
				raise(SIGTRAP);
			}
#elif defined(_MSC_VER)
			__debugbreak();
#endif
		}
	}
}
/// <summary>Convert Vulkan error code to string</summary>
/// <param name="errorCode">Vulkan error</param>
/// <returns>Error string</returns>
inline char const* vkErrorToStr(Result errorCode)
{
	switch (errorCode)
	{
	case Result::e_SUCCESS: return "VK_SUCCESS";
	case Result::e_NOT_READY: return "VK_NOT_READY";
	case Result::e_TIMEOUT: return "VK_TIMEOUT";
	case Result::e_EVENT_SET: return "VK_EVENT_SET";
	case Result::e_EVENT_RESET: return "VK_EVENT_RESET";
	case Result::e_INCOMPLETE: return "VK_INCOMPLETE";
	case Result::e_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
	case Result::e_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case Result::e_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case Result::e_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
	case Result::e_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
	case Result::e_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
	case Result::e_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
	case Result::e_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case Result::e_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
	case Result::e_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case Result::e_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
	case Result::e_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case Result::e_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
	case Result::e_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
	case Result::e_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
	case Result::e_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case Result::e_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
	case Result::e_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case Result::e_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
	case Result::e_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
	case Result::e_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case Result::e_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case Result::e_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case Result::e_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
	case Result::e_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
	case Result::e_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case Result::e_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
	default: return ("-- Result::UNKNOWN ERROR CODE--");
		break;
	}
}

class Error: public std::runtime_error
{
protected:
	Result _result;
public:
	virtual ~Error() {}
	Error(Result result, const std::string& errorMessage) :
		runtime_error(std::string("[") + vkErrorToStr(result) + ("] : ") + errorMessage), _result(result)
	{
#ifdef DEBUG
		impl::debuggerBreak();
#endif
	}
	Error(Result result, const char* errorMessage = nullptr) :
		runtime_error((std::string("[") + vkErrorToStr(result) + "] : ") + (errorMessage ? errorMessage : "")), _result(result)
	{
#ifdef DEBUG
		impl::debuggerBreak();
#endif
	}
	const char* getErrorMessage() const { return what(); }
	Result getResult() const { return _result; }
	const char* getResultCode() const { return vkErrorToStr(_result); }
};

class Success: public Error
{
public:
	virtual ~Success(){}
	Success(const char* errorMessage = nullptr): Error(Result::e_SUCCESS, errorMessage) {}
	explicit Success(const std::string& errorMessage): Error(Result::e_SUCCESS, errorMessage) {}
};
class NotReady: public Error
{
public:
	virtual ~NotReady(){}
	NotReady(const char* errorMessage = nullptr): Error(Result::e_NOT_READY, errorMessage) {}
	explicit NotReady(const std::string& errorMessage): Error(Result::e_NOT_READY, errorMessage) {}
};
class Timeout: public Error
{
public:
	virtual ~Timeout(){}
	Timeout(const char* errorMessage = nullptr): Error(Result::e_TIMEOUT, errorMessage) {}
	explicit Timeout(const std::string& errorMessage): Error(Result::e_TIMEOUT, errorMessage) {}
};
class EventSet: public Error
{
public:
	virtual ~EventSet(){}
	EventSet(const char* errorMessage = nullptr): Error(Result::e_EVENT_SET, errorMessage) {}
	explicit EventSet(const std::string& errorMessage): Error(Result::e_EVENT_SET, errorMessage) {}
};
class EventReset: public Error
{
public:
	virtual ~EventReset(){}
	EventReset(const char* errorMessage = nullptr): Error(Result::e_EVENT_RESET, errorMessage) {}
	explicit EventReset(const std::string& errorMessage): Error(Result::e_EVENT_RESET, errorMessage) {}
};
class Incomplete: public Error
{
public:
	virtual ~Incomplete(){}
	Incomplete(const char* errorMessage = nullptr): Error(Result::e_INCOMPLETE, errorMessage) {}
	explicit Incomplete(const std::string& errorMessage): Error(Result::e_INCOMPLETE, errorMessage) {}
};
class SuboptimalKHR: public Error
{
public:
	virtual ~SuboptimalKHR(){}
	SuboptimalKHR(const char* errorMessage = nullptr): Error(Result::e_SUBOPTIMAL_KHR, errorMessage) {}
	explicit SuboptimalKHR(const std::string& errorMessage): Error(Result::e_SUBOPTIMAL_KHR, errorMessage) {}
};
class ErrorOutOfHostMemory: public Error
{
public:
	virtual ~ErrorOutOfHostMemory(){}
	ErrorOutOfHostMemory(const char* errorMessage = nullptr): Error(Result::e_ERROR_OUT_OF_HOST_MEMORY, errorMessage) {}
	explicit ErrorOutOfHostMemory(const std::string& errorMessage): Error(Result::e_ERROR_OUT_OF_HOST_MEMORY, errorMessage) {}
};
class ErrorOutOfDeviceMemory: public Error
{
public:
	virtual ~ErrorOutOfDeviceMemory(){}
	ErrorOutOfDeviceMemory(const char* errorMessage = nullptr): Error(Result::e_ERROR_OUT_OF_DEVICE_MEMORY, errorMessage) {}
	explicit ErrorOutOfDeviceMemory(const std::string& errorMessage): Error(Result::e_ERROR_OUT_OF_DEVICE_MEMORY, errorMessage) {}
};
class ErrorInitializationFailed: public Error
{
public:
	virtual ~ErrorInitializationFailed(){}
	ErrorInitializationFailed(const char* errorMessage = nullptr): Error(Result::e_ERROR_INITIALIZATION_FAILED, errorMessage) {}
	explicit ErrorInitializationFailed(const std::string& errorMessage): Error(Result::e_ERROR_INITIALIZATION_FAILED, errorMessage) {}
};
class ErrorDeviceLost: public Error
{
public:
	virtual ~ErrorDeviceLost(){}
	ErrorDeviceLost(const char* errorMessage = nullptr): Error(Result::e_ERROR_DEVICE_LOST, errorMessage) {}
	explicit ErrorDeviceLost(const std::string& errorMessage): Error(Result::e_ERROR_DEVICE_LOST, errorMessage) {}
};
class ErrorMemoryMapFailed: public Error
{
public:
	virtual ~ErrorMemoryMapFailed(){}
	ErrorMemoryMapFailed(const char* errorMessage = nullptr): Error(Result::e_ERROR_MEMORY_MAP_FAILED, errorMessage) {}
	explicit ErrorMemoryMapFailed(const std::string& errorMessage): Error(Result::e_ERROR_MEMORY_MAP_FAILED, errorMessage) {}
};
class ErrorLayerNotPresent: public Error
{
public:
	virtual ~ErrorLayerNotPresent(){}
	ErrorLayerNotPresent(const char* errorMessage = nullptr): Error(Result::e_ERROR_LAYER_NOT_PRESENT, errorMessage) {}
	explicit ErrorLayerNotPresent(const std::string& errorMessage): Error(Result::e_ERROR_LAYER_NOT_PRESENT, errorMessage) {}
};
class ErrorExtensionNotPresent: public Error
{
public:
	virtual ~ErrorExtensionNotPresent(){}
	ErrorExtensionNotPresent(const char* errorMessage = nullptr): Error(Result::e_ERROR_EXTENSION_NOT_PRESENT, errorMessage) {}
	explicit ErrorExtensionNotPresent(const std::string& errorMessage): Error(Result::e_ERROR_EXTENSION_NOT_PRESENT, errorMessage) {}
};
class ErrorFeatureNotPresent: public Error
{
public:
	virtual ~ErrorFeatureNotPresent(){}
	ErrorFeatureNotPresent(const char* errorMessage = nullptr): Error(Result::e_ERROR_FEATURE_NOT_PRESENT, errorMessage) {}
	explicit ErrorFeatureNotPresent(const std::string& errorMessage): Error(Result::e_ERROR_FEATURE_NOT_PRESENT, errorMessage) {}
};
class ErrorIncompatibleDriver: public Error
{
public:
	virtual ~ErrorIncompatibleDriver(){}
	ErrorIncompatibleDriver(const char* errorMessage = nullptr): Error(Result::e_ERROR_INCOMPATIBLE_DRIVER, errorMessage) {}
	explicit ErrorIncompatibleDriver(const std::string& errorMessage): Error(Result::e_ERROR_INCOMPATIBLE_DRIVER, errorMessage) {}
};
class ErrorTooManyObjects: public Error
{
public:
	virtual ~ErrorTooManyObjects(){}
	ErrorTooManyObjects(const char* errorMessage = nullptr): Error(Result::e_ERROR_TOO_MANY_OBJECTS, errorMessage) {}
	explicit ErrorTooManyObjects(const std::string& errorMessage): Error(Result::e_ERROR_TOO_MANY_OBJECTS, errorMessage) {}
};
class ErrorFormatNotSupported: public Error
{
public:
	virtual ~ErrorFormatNotSupported(){}
	ErrorFormatNotSupported(const char* errorMessage = nullptr): Error(Result::e_ERROR_FORMAT_NOT_SUPPORTED, errorMessage) {}
	explicit ErrorFormatNotSupported(const std::string& errorMessage): Error(Result::e_ERROR_FORMAT_NOT_SUPPORTED, errorMessage) {}
};
class ErrorFragmentedPool: public Error
{
public:
	virtual ~ErrorFragmentedPool(){}
	ErrorFragmentedPool(const char* errorMessage = nullptr): Error(Result::e_ERROR_FRAGMENTED_POOL, errorMessage) {}
	explicit ErrorFragmentedPool(const std::string& errorMessage): Error(Result::e_ERROR_FRAGMENTED_POOL, errorMessage) {}
};
class ErrorUnknown: public Error
{
public:
	virtual ~ErrorUnknown(){}
	ErrorUnknown(const char* errorMessage = nullptr): Error(Result::e_ERROR_UNKNOWN, errorMessage) {}
	explicit ErrorUnknown(const std::string& errorMessage): Error(Result::e_ERROR_UNKNOWN, errorMessage) {}
};
class ErrorSurfaceLostKHR: public Error
{
public:
	virtual ~ErrorSurfaceLostKHR(){}
	ErrorSurfaceLostKHR(const char* errorMessage = nullptr): Error(Result::e_ERROR_SURFACE_LOST_KHR, errorMessage) {}
	explicit ErrorSurfaceLostKHR(const std::string& errorMessage): Error(Result::e_ERROR_SURFACE_LOST_KHR, errorMessage) {}
};
class ErrorNativeWindowInUseKHR: public Error
{
public:
	virtual ~ErrorNativeWindowInUseKHR(){}
	ErrorNativeWindowInUseKHR(const char* errorMessage = nullptr): Error(Result::e_ERROR_NATIVE_WINDOW_IN_USE_KHR, errorMessage) {}
	explicit ErrorNativeWindowInUseKHR(const std::string& errorMessage): Error(Result::e_ERROR_NATIVE_WINDOW_IN_USE_KHR, errorMessage) {}
};
class ErrorOutOfDateKHR: public Error
{
public:
	virtual ~ErrorOutOfDateKHR(){}
	ErrorOutOfDateKHR(const char* errorMessage = nullptr): Error(Result::e_ERROR_OUT_OF_DATE_KHR, errorMessage) {}
	explicit ErrorOutOfDateKHR(const std::string& errorMessage): Error(Result::e_ERROR_OUT_OF_DATE_KHR, errorMessage) {}
};
class ErrorIncompatibleDisplayKHR: public Error
{
public:
	virtual ~ErrorIncompatibleDisplayKHR(){}
	ErrorIncompatibleDisplayKHR(const char* errorMessage = nullptr): Error(Result::e_ERROR_INCOMPATIBLE_DISPLAY_KHR, errorMessage) {}
	explicit ErrorIncompatibleDisplayKHR(const std::string& errorMessage): Error(Result::e_ERROR_INCOMPATIBLE_DISPLAY_KHR, errorMessage) {}
};
class ErrorValidationFailedEXT: public Error
{
public:
	virtual ~ErrorValidationFailedEXT(){}
	ErrorValidationFailedEXT(const char* errorMessage = nullptr): Error(Result::e_ERROR_VALIDATION_FAILED_EXT, errorMessage) {}
	explicit ErrorValidationFailedEXT(const std::string& errorMessage): Error(Result::e_ERROR_VALIDATION_FAILED_EXT, errorMessage) {}
};
class ErrorInvalidShaderNV: public Error
{
public:
	virtual ~ErrorInvalidShaderNV(){}
	ErrorInvalidShaderNV(const char* errorMessage = nullptr): Error(Result::e_ERROR_INVALID_SHADER_NV, errorMessage) {}
	explicit ErrorInvalidShaderNV(const std::string& errorMessage): Error(Result::e_ERROR_INVALID_SHADER_NV, errorMessage) {}
};
class ErrorOutOfPoolMemory: public Error
{
public:
	virtual ~ErrorOutOfPoolMemory(){}
	ErrorOutOfPoolMemory(const char* errorMessage = nullptr): Error(Result::e_ERROR_OUT_OF_POOL_MEMORY, errorMessage) {}
	explicit ErrorOutOfPoolMemory(const std::string& errorMessage): Error(Result::e_ERROR_OUT_OF_POOL_MEMORY, errorMessage) {}
};
class ErrorInvalidExternalHandle: public Error
{
public:
	virtual ~ErrorInvalidExternalHandle(){}
	ErrorInvalidExternalHandle(const char* errorMessage = nullptr): Error(Result::e_ERROR_INVALID_EXTERNAL_HANDLE, errorMessage) {}
	explicit ErrorInvalidExternalHandle(const std::string& errorMessage): Error(Result::e_ERROR_INVALID_EXTERNAL_HANDLE, errorMessage) {}
};
class ErrorInvalidDrmFormatModifierPlaneLayoutEXT: public Error
{
public:
	virtual ~ErrorInvalidDrmFormatModifierPlaneLayoutEXT(){}
	ErrorInvalidDrmFormatModifierPlaneLayoutEXT(const char* errorMessage = nullptr): Error(Result::e_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT, errorMessage) {}
	explicit ErrorInvalidDrmFormatModifierPlaneLayoutEXT(const std::string& errorMessage): Error(Result::e_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT, errorMessage) {}
};
class ErrorFragmentation: public Error
{
public:
	virtual ~ErrorFragmentation(){}
	ErrorFragmentation(const char* errorMessage = nullptr): Error(Result::e_ERROR_FRAGMENTATION, errorMessage) {}
	explicit ErrorFragmentation(const std::string& errorMessage): Error(Result::e_ERROR_FRAGMENTATION, errorMessage) {}
};
class ErrorNotPermittedEXT: public Error
{
public:
	virtual ~ErrorNotPermittedEXT(){}
	ErrorNotPermittedEXT(const char* errorMessage = nullptr): Error(Result::e_ERROR_NOT_PERMITTED_EXT, errorMessage) {}
	explicit ErrorNotPermittedEXT(const std::string& errorMessage): Error(Result::e_ERROR_NOT_PERMITTED_EXT, errorMessage) {}
};
class ErrorFullScreenExclusiveModeLostEXT: public Error
{
public:
	virtual ~ErrorFullScreenExclusiveModeLostEXT(){}
	ErrorFullScreenExclusiveModeLostEXT(const char* errorMessage = nullptr): Error(Result::e_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, errorMessage) {}
	explicit ErrorFullScreenExclusiveModeLostEXT(const std::string& errorMessage): Error(Result::e_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, errorMessage) {}
};
class ErrorInvalidOpaqueCaptureAddress: public Error
{
public:
	virtual ~ErrorInvalidOpaqueCaptureAddress(){}
	ErrorInvalidOpaqueCaptureAddress(const char* errorMessage = nullptr): Error(Result::e_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS, errorMessage) {}
	explicit ErrorInvalidOpaqueCaptureAddress(const std::string& errorMessage): Error(Result::e_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS, errorMessage) {}
};

namespace impl {
/// <summary>Throw a Vulkan exception if the result is not successful.</summary>
/// <param name="result">A vulkan result code</param>
/// <param name="msg">Throw a corresponding exception if the error code is not success</param>
inline void vkThrowIfFailed(Result result, const char* message = 0)
{
	switch (result)
	{
	case Result::e_NOT_READY: throw NotReady(message);
	case Result::e_TIMEOUT: throw Timeout(message);
	case Result::e_EVENT_SET: throw EventSet(message);
	case Result::e_EVENT_RESET: throw EventReset(message);
	case Result::e_INCOMPLETE: throw Incomplete(message);
	case Result::e_SUBOPTIMAL_KHR: throw SuboptimalKHR(message);
	case Result::e_ERROR_OUT_OF_HOST_MEMORY: throw ErrorOutOfHostMemory(message);
	case Result::e_ERROR_OUT_OF_DEVICE_MEMORY: throw ErrorOutOfDeviceMemory(message);
	case Result::e_ERROR_INITIALIZATION_FAILED: throw ErrorInitializationFailed(message);
	case Result::e_ERROR_DEVICE_LOST: throw ErrorDeviceLost(message);
	case Result::e_ERROR_MEMORY_MAP_FAILED: throw ErrorMemoryMapFailed(message);
	case Result::e_ERROR_LAYER_NOT_PRESENT: throw ErrorLayerNotPresent(message);
	case Result::e_ERROR_EXTENSION_NOT_PRESENT: throw ErrorExtensionNotPresent(message);
	case Result::e_ERROR_FEATURE_NOT_PRESENT: throw ErrorFeatureNotPresent(message);
	case Result::e_ERROR_INCOMPATIBLE_DRIVER: throw ErrorIncompatibleDriver(message);
	case Result::e_ERROR_TOO_MANY_OBJECTS: throw ErrorTooManyObjects(message);
	case Result::e_ERROR_FORMAT_NOT_SUPPORTED: throw ErrorFormatNotSupported(message);
	case Result::e_ERROR_FRAGMENTED_POOL: throw ErrorFragmentedPool(message);
	case Result::e_ERROR_UNKNOWN: throw ErrorUnknown(message);
	case Result::e_ERROR_SURFACE_LOST_KHR: throw ErrorSurfaceLostKHR(message);
	case Result::e_ERROR_NATIVE_WINDOW_IN_USE_KHR: throw ErrorNativeWindowInUseKHR(message);
	case Result::e_ERROR_OUT_OF_DATE_KHR: throw ErrorOutOfDateKHR(message);
	case Result::e_ERROR_INCOMPATIBLE_DISPLAY_KHR: throw ErrorIncompatibleDisplayKHR(message);
	case Result::e_ERROR_VALIDATION_FAILED_EXT: throw ErrorValidationFailedEXT(message);
	case Result::e_ERROR_INVALID_SHADER_NV: throw ErrorInvalidShaderNV(message);
	case Result::e_ERROR_OUT_OF_POOL_MEMORY: throw ErrorOutOfPoolMemory(message);
	case Result::e_ERROR_INVALID_EXTERNAL_HANDLE: throw ErrorInvalidExternalHandle(message);
	case Result::e_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: throw ErrorInvalidDrmFormatModifierPlaneLayoutEXT(message);
	case Result::e_ERROR_FRAGMENTATION: throw ErrorFragmentation(message);
	case Result::e_ERROR_NOT_PERMITTED_EXT: throw ErrorNotPermittedEXT(message);
	case Result::e_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: throw ErrorFullScreenExclusiveModeLostEXT(message);
	case Result::e_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: throw ErrorInvalidOpaqueCaptureAddress(message);
	case Result::e_SUCCESS:
		break;
	default: throw ErrorUnknown(message);
	}
}

/// <summary>Throw a Vulkan exception if the result is not successful.</summary>
/// <param name="result">A vulkan result code</param>
/// <param name="msg">Throw a corresponding exception if the error code is not success</param>
inline void vkThrowIfFailed(VkResult result, const char* message = 0)
{
	vkThrowIfFailed(static_cast<pvrvk::Result>(result), message);
}

/// <summary>Throw a Vulkan exception if the result is not successful.</summary>
/// <param name="result">A vulkan result code</param>
/// <param name="msg">Throw a corresponding exception if the error code is an error</param>
inline void vkThrowIfError(Result result, const char* message = 0)
{
	switch (result)
	{
	case Result::e_ERROR_OUT_OF_HOST_MEMORY: throw ErrorOutOfHostMemory(message);
	case Result::e_ERROR_OUT_OF_DEVICE_MEMORY: throw ErrorOutOfDeviceMemory(message);
	case Result::e_ERROR_INITIALIZATION_FAILED: throw ErrorInitializationFailed(message);
	case Result::e_ERROR_DEVICE_LOST: throw ErrorDeviceLost(message);
	case Result::e_ERROR_MEMORY_MAP_FAILED: throw ErrorMemoryMapFailed(message);
	case Result::e_ERROR_LAYER_NOT_PRESENT: throw ErrorLayerNotPresent(message);
	case Result::e_ERROR_EXTENSION_NOT_PRESENT: throw ErrorExtensionNotPresent(message);
	case Result::e_ERROR_FEATURE_NOT_PRESENT: throw ErrorFeatureNotPresent(message);
	case Result::e_ERROR_INCOMPATIBLE_DRIVER: throw ErrorIncompatibleDriver(message);
	case Result::e_ERROR_TOO_MANY_OBJECTS: throw ErrorTooManyObjects(message);
	case Result::e_ERROR_FORMAT_NOT_SUPPORTED: throw ErrorFormatNotSupported(message);
	case Result::e_ERROR_FRAGMENTED_POOL: throw ErrorFragmentedPool(message);
	case Result::e_ERROR_UNKNOWN: throw ErrorUnknown(message);
	case Result::e_ERROR_SURFACE_LOST_KHR: throw ErrorSurfaceLostKHR(message);
	case Result::e_ERROR_NATIVE_WINDOW_IN_USE_KHR: throw ErrorNativeWindowInUseKHR(message);
	case Result::e_ERROR_OUT_OF_DATE_KHR: throw ErrorOutOfDateKHR(message);
	case Result::e_ERROR_INCOMPATIBLE_DISPLAY_KHR: throw ErrorIncompatibleDisplayKHR(message);
	case Result::e_ERROR_VALIDATION_FAILED_EXT: throw ErrorValidationFailedEXT(message);
	case Result::e_ERROR_INVALID_SHADER_NV: throw ErrorInvalidShaderNV(message);
	case Result::e_ERROR_OUT_OF_POOL_MEMORY: throw ErrorOutOfPoolMemory(message);
	case Result::e_ERROR_INVALID_EXTERNAL_HANDLE: throw ErrorInvalidExternalHandle(message);
	case Result::e_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: throw ErrorInvalidDrmFormatModifierPlaneLayoutEXT(message);
	case Result::e_ERROR_FRAGMENTATION: throw ErrorFragmentation(message);
	case Result::e_ERROR_NOT_PERMITTED_EXT: throw ErrorNotPermittedEXT(message);
	case Result::e_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: throw ErrorFullScreenExclusiveModeLostEXT(message);
	case Result::e_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: throw ErrorInvalidOpaqueCaptureAddress(message);
	case Result::e_SUCCESS:
	case Result::e_NOT_READY:
	case Result::e_TIMEOUT:
	case Result::e_EVENT_SET:
	case Result::e_EVENT_RESET:
	case Result::e_INCOMPLETE:
	case Result::e_SUBOPTIMAL_KHR:
		break;
	default: throw ErrorUnknown(message);
	}
}
}// namespace impl


// Generated extension tables which can be used to determine support for enabled extensions
/// <summary>Instance extension table.</summary>
struct InstanceExtensionTable
{
public:
	// VK_KHR_surface
	bool khrSurfaceEnabled = false;
	// VK_KHR_display
	bool khrDisplayEnabled = false;
	// VK_KHR_xlib_surface
	bool khrXlibSurfaceEnabled = false;
	// VK_KHR_xcb_surface
	bool khrXcbSurfaceEnabled = false;
	// VK_KHR_wayland_surface
	bool khrWaylandSurfaceEnabled = false;
	// VK_KHR_android_surface
	bool khrAndroidSurfaceEnabled = false;
	// VK_KHR_win32_surface
	bool khrWin32SurfaceEnabled = false;
	// VK_EXT_debug_report
	bool extDebugReportEnabled = false;
	// VK_GGP_stream_descriptor_surface
	bool ggpStreamDescriptorSurfaceEnabled = false;
	// VK_NV_external_memory_capabilities
	bool nvExternalMemoryCapabilitiesEnabled = false;
	// VK_KHR_get_physical_device_properties2
	bool khrGetPhysicalDeviceProperties2Enabled = false;
	// VK_EXT_validation_flags
	bool extValidationFlagsEnabled = false;
	// VK_NN_vi_surface
	bool nnViSurfaceEnabled = false;
	// VK_KHR_device_group_creation
	bool khrDeviceGroupCreationEnabled = false;
	// VK_KHR_external_memory_capabilities
	bool khrExternalMemoryCapabilitiesEnabled = false;
	// VK_KHR_external_semaphore_capabilities
	bool khrExternalSemaphoreCapabilitiesEnabled = false;
	// VK_EXT_direct_mode_display
	bool extDirectModeDisplayEnabled = false;
	// VK_EXT_acquire_xlib_display
	bool extAcquireXlibDisplayEnabled = false;
	// VK_EXT_display_surface_counter
	bool extDisplaySurfaceCounterEnabled = false;
	// VK_EXT_swapchain_colorspace
	bool extSwapchainColorspaceEnabled = false;
	// VK_KHR_external_fence_capabilities
	bool khrExternalFenceCapabilitiesEnabled = false;
	// VK_KHR_get_surface_capabilities2
	bool khrGetSurfaceCapabilities2Enabled = false;
	// VK_KHR_get_display_properties2
	bool khrGetDisplayProperties2Enabled = false;
	// VK_MVK_ios_surface
	bool mvkIosSurfaceEnabled = false;
	// VK_MVK_macos_surface
	bool mvkMacosSurfaceEnabled = false;
	// VK_EXT_debug_utils
	bool extDebugUtilsEnabled = false;
	// VK_FUCHSIA_imagepipe_surface
	bool fuchsiaImagepipeSurfaceEnabled = false;
	// VK_EXT_metal_surface
	bool extMetalSurfaceEnabled = false;
	// VK_KHR_surface_protected_capabilities
	bool khrSurfaceProtectedCapabilitiesEnabled = false;
	// VK_EXT_validation_features
	bool extValidationFeaturesEnabled = false;
	// VK_EXT_headless_surface
	bool extHeadlessSurfaceEnabled = false;

void setEnabledExtension(const std::string& extension)
{
	std::string instanceExtensions[31] = {
		"VK_KHR_surface",
		"VK_KHR_display",
		"VK_KHR_xlib_surface",
		"VK_KHR_xcb_surface",
		"VK_KHR_wayland_surface",
		"VK_KHR_android_surface",
		"VK_KHR_win32_surface",
		"VK_EXT_debug_report",
		"VK_GGP_stream_descriptor_surface",
		"VK_NV_external_memory_capabilities",
		"VK_KHR_get_physical_device_properties2",
		"VK_EXT_validation_flags",
		"VK_NN_vi_surface",
		"VK_KHR_device_group_creation",
		"VK_KHR_external_memory_capabilities",
		"VK_KHR_external_semaphore_capabilities",
		"VK_EXT_direct_mode_display",
		"VK_EXT_acquire_xlib_display",
		"VK_EXT_display_surface_counter",
		"VK_EXT_swapchain_colorspace",
		"VK_KHR_external_fence_capabilities",
		"VK_KHR_get_surface_capabilities2",
		"VK_KHR_get_display_properties2",
		"VK_MVK_ios_surface",
		"VK_MVK_macos_surface",
		"VK_EXT_debug_utils",
		"VK_FUCHSIA_imagepipe_surface",
		"VK_EXT_metal_surface",
		"VK_KHR_surface_protected_capabilities",
		"VK_EXT_validation_features",
		"VK_EXT_headless_surface",
	};

	bool* instanceExtensionsFlags[31] = {
		&khrSurfaceEnabled,
		&khrDisplayEnabled,
		&khrXlibSurfaceEnabled,
		&khrXcbSurfaceEnabled,
		&khrWaylandSurfaceEnabled,
		&khrAndroidSurfaceEnabled,
		&khrWin32SurfaceEnabled,
		&extDebugReportEnabled,
		&ggpStreamDescriptorSurfaceEnabled,
		&nvExternalMemoryCapabilitiesEnabled,
		&khrGetPhysicalDeviceProperties2Enabled,
		&extValidationFlagsEnabled,
		&nnViSurfaceEnabled,
		&khrDeviceGroupCreationEnabled,
		&khrExternalMemoryCapabilitiesEnabled,
		&khrExternalSemaphoreCapabilitiesEnabled,
		&extDirectModeDisplayEnabled,
		&extAcquireXlibDisplayEnabled,
		&extDisplaySurfaceCounterEnabled,
		&extSwapchainColorspaceEnabled,
		&khrExternalFenceCapabilitiesEnabled,
		&khrGetSurfaceCapabilities2Enabled,
		&khrGetDisplayProperties2Enabled,
		&mvkIosSurfaceEnabled,
		&mvkMacosSurfaceEnabled,
		&extDebugUtilsEnabled,
		&fuchsiaImagepipeSurfaceEnabled,
		&extMetalSurfaceEnabled,
		&khrSurfaceProtectedCapabilitiesEnabled,
		&extValidationFeaturesEnabled,
		&extHeadlessSurfaceEnabled,
	};

	for(uint32_t i = 0; i < 31; ++i)
	{
		if (extension == instanceExtensions[i])
		{
			*instanceExtensionsFlags[i] = true;
		}
	}
}

void setEnabledExtensions(const std::vector<std::string>& extensions)
{
	for(uint32_t i = 0; i < static_cast<uint32_t>(extensions.size()); ++i)
	{
		setEnabledExtension(extensions[i]);
	}
}

void setEnabledExtensions(const std::vector<const char*>& extensions)
{
	for(uint32_t i = 0; i < static_cast<uint32_t>(extensions.size()); ++i)
	{
		setEnabledExtension(extensions[i]);
	}
}
};

/// <summary>Device extension table.</summary>
struct DeviceExtensionTable
{
public:
	// VK_KHR_swapchain
	bool khrSwapchainEnabled = false;
	// VK_KHR_display_swapchain
	bool khrDisplaySwapchainEnabled = false;
	// VK_NV_glsl_shader
	bool nvGlslShaderEnabled = false;
	// VK_EXT_depth_range_unrestricted
	bool extDepthRangeUnrestrictedEnabled = false;
	// VK_KHR_sampler_mirror_clamp_to_edge
	bool khrSamplerMirrorClampToEdgeEnabled = false;
	// VK_IMG_filter_cubic
	bool imgFilterCubicEnabled = false;
	// VK_AMD_rasterization_order
	bool amdRasterizationOrderEnabled = false;
	// VK_AMD_shader_trinary_minmax
	bool amdShaderTrinaryMinmaxEnabled = false;
	// VK_AMD_shader_explicit_vertex_parameter
	bool amdShaderExplicitVertexParameterEnabled = false;
	// VK_EXT_debug_marker
	bool extDebugMarkerEnabled = false;
	// VK_AMD_gcn_shader
	bool amdGcnShaderEnabled = false;
	// VK_NV_dedicated_allocation
	bool nvDedicatedAllocationEnabled = false;
	// VK_EXT_transform_feedback
	bool extTransformFeedbackEnabled = false;
	// VK_NVX_image_view_handle
	bool nvxImageViewHandleEnabled = false;
	// VK_AMD_draw_indirect_count
	bool amdDrawIndirectCountEnabled = false;
	// VK_AMD_negative_viewport_height
	bool amdNegativeViewportHeightEnabled = false;
	// VK_AMD_gpu_shader_half_float
	bool amdGpuShaderHalfFloatEnabled = false;
	// VK_AMD_shader_ballot
	bool amdShaderBallotEnabled = false;
	// VK_AMD_texture_gather_bias_lod
	bool amdTextureGatherBiasLodEnabled = false;
	// VK_AMD_shader_info
	bool amdShaderInfoEnabled = false;
	// VK_AMD_shader_image_load_store_lod
	bool amdShaderImageLoadStoreLodEnabled = false;
	// VK_NV_corner_sampled_image
	bool nvCornerSampledImageEnabled = false;
	// VK_KHR_multiview
	bool khrMultiviewEnabled = false;
	// VK_IMG_format_pvrtc
	bool imgFormatPvrtcEnabled = false;
	// VK_NV_external_memory
	bool nvExternalMemoryEnabled = false;
	// VK_NV_external_memory_win32
	bool nvExternalMemoryWin32Enabled = false;
	// VK_NV_win32_keyed_mutex
	bool nvWin32KeyedMutexEnabled = false;
	// VK_KHR_device_group
	bool khrDeviceGroupEnabled = false;
	// VK_KHR_shader_draw_parameters
	bool khrShaderDrawParametersEnabled = false;
	// VK_EXT_shader_subgroup_ballot
	bool extShaderSubgroupBallotEnabled = false;
	// VK_EXT_shader_subgroup_vote
	bool extShaderSubgroupVoteEnabled = false;
	// VK_EXT_texture_compression_astc_hdr
	bool extTextureCompressionAstcHdrEnabled = false;
	// VK_EXT_astc_decode_mode
	bool extAstcDecodeModeEnabled = false;
	// VK_KHR_maintenance1
	bool khrMaintenance1Enabled = false;
	// VK_KHR_external_memory
	bool khrExternalMemoryEnabled = false;
	// VK_KHR_external_memory_win32
	bool khrExternalMemoryWin32Enabled = false;
	// VK_KHR_external_memory_fd
	bool khrExternalMemoryFdEnabled = false;
	// VK_KHR_win32_keyed_mutex
	bool khrWin32KeyedMutexEnabled = false;
	// VK_KHR_external_semaphore
	bool khrExternalSemaphoreEnabled = false;
	// VK_KHR_external_semaphore_win32
	bool khrExternalSemaphoreWin32Enabled = false;
	// VK_KHR_external_semaphore_fd
	bool khrExternalSemaphoreFdEnabled = false;
	// VK_KHR_push_descriptor
	bool khrPushDescriptorEnabled = false;
	// VK_EXT_conditional_rendering
	bool extConditionalRenderingEnabled = false;
	// VK_KHR_shader_float16_int8
	bool khrShaderFloat16Int8Enabled = false;
	// VK_KHR_16bit_storage
	bool khr16BitStorageEnabled = false;
	// VK_KHR_incremental_present
	bool khrIncrementalPresentEnabled = false;
	// VK_KHR_descriptor_update_template
	bool khrDescriptorUpdateTemplateEnabled = false;
	// VK_NVX_device_generated_commands
	bool nvxDeviceGeneratedCommandsEnabled = false;
	// VK_NV_clip_space_w_scaling
	bool nvClipSpaceWScalingEnabled = false;
	// VK_EXT_display_control
	bool extDisplayControlEnabled = false;
	// VK_GOOGLE_display_timing
	bool googleDisplayTimingEnabled = false;
	// VK_NV_sample_mask_override_coverage
	bool nvSampleMaskOverrideCoverageEnabled = false;
	// VK_NV_geometry_shader_passthrough
	bool nvGeometryShaderPassthroughEnabled = false;
	// VK_NV_viewport_array2
	bool nvViewportArray2Enabled = false;
	// VK_NVX_multiview_per_view_attributes
	bool nvxMultiviewPerViewAttributesEnabled = false;
	// VK_NV_viewport_swizzle
	bool nvViewportSwizzleEnabled = false;
	// VK_EXT_discard_rectangles
	bool extDiscardRectanglesEnabled = false;
	// VK_EXT_conservative_rasterization
	bool extConservativeRasterizationEnabled = false;
	// VK_EXT_depth_clip_enable
	bool extDepthClipEnableEnabled = false;
	// VK_EXT_hdr_metadata
	bool extHdrMetadataEnabled = false;
	// VK_KHR_imageless_framebuffer
	bool khrImagelessFramebufferEnabled = false;
	// VK_KHR_create_renderpass2
	bool khrCreateRenderpass2Enabled = false;
	// VK_KHR_shared_presentable_image
	bool khrSharedPresentableImageEnabled = false;
	// VK_KHR_external_fence
	bool khrExternalFenceEnabled = false;
	// VK_KHR_external_fence_win32
	bool khrExternalFenceWin32Enabled = false;
	// VK_KHR_external_fence_fd
	bool khrExternalFenceFdEnabled = false;
	// VK_KHR_performance_query
	bool khrPerformanceQueryEnabled = false;
	// VK_KHR_maintenance2
	bool khrMaintenance2Enabled = false;
	// VK_KHR_variable_pointers
	bool khrVariablePointersEnabled = false;
	// VK_EXT_external_memory_dma_buf
	bool extExternalMemoryDmaBufEnabled = false;
	// VK_EXT_queue_family_foreign
	bool extQueueFamilyForeignEnabled = false;
	// VK_KHR_dedicated_allocation
	bool khrDedicatedAllocationEnabled = false;
	// VK_ANDROID_external_memory_android_hardware_buffer
	bool androidExternalMemoryAndroidHardwareBufferEnabled = false;
	// VK_EXT_sampler_filter_minmax
	bool extSamplerFilterMinmaxEnabled = false;
	// VK_KHR_storage_buffer_storage_class
	bool khrStorageBufferStorageClassEnabled = false;
	// VK_AMD_gpu_shader_int16
	bool amdGpuShaderInt16Enabled = false;
	// VK_AMD_mixed_attachment_samples
	bool amdMixedAttachmentSamplesEnabled = false;
	// VK_AMD_shader_fragment_mask
	bool amdShaderFragmentMaskEnabled = false;
	// VK_EXT_inline_uniform_block
	bool extInlineUniformBlockEnabled = false;
	// VK_EXT_shader_stencil_export
	bool extShaderStencilExportEnabled = false;
	// VK_EXT_sample_locations
	bool extSampleLocationsEnabled = false;
	// VK_KHR_relaxed_block_layout
	bool khrRelaxedBlockLayoutEnabled = false;
	// VK_KHR_get_memory_requirements2
	bool khrGetMemoryRequirements2Enabled = false;
	// VK_KHR_image_format_list
	bool khrImageFormatListEnabled = false;
	// VK_EXT_blend_operation_advanced
	bool extBlendOperationAdvancedEnabled = false;
	// VK_NV_fragment_coverage_to_color
	bool nvFragmentCoverageToColorEnabled = false;
	// VK_NV_framebuffer_mixed_samples
	bool nvFramebufferMixedSamplesEnabled = false;
	// VK_NV_fill_rectangle
	bool nvFillRectangleEnabled = false;
	// VK_NV_shader_sm_builtins
	bool nvShaderSmBuiltinsEnabled = false;
	// VK_EXT_post_depth_coverage
	bool extPostDepthCoverageEnabled = false;
	// VK_KHR_sampler_ycbcr_conversion
	bool khrSamplerYcbcrConversionEnabled = false;
	// VK_KHR_bind_memory2
	bool khrBindMemory2Enabled = false;
	// VK_EXT_image_drm_format_modifier
	bool extImageDrmFormatModifierEnabled = false;
	// VK_EXT_validation_cache
	bool extValidationCacheEnabled = false;
	// VK_EXT_descriptor_indexing
	bool extDescriptorIndexingEnabled = false;
	// VK_EXT_shader_viewport_index_layer
	bool extShaderViewportIndexLayerEnabled = false;
	// VK_NV_shading_rate_image
	bool nvShadingRateImageEnabled = false;
	// VK_NV_ray_tracing
	bool nvRayTracingEnabled = false;
	// VK_NV_representative_fragment_test
	bool nvRepresentativeFragmentTestEnabled = false;
	// VK_KHR_maintenance3
	bool khrMaintenance3Enabled = false;
	// VK_KHR_draw_indirect_count
	bool khrDrawIndirectCountEnabled = false;
	// VK_EXT_filter_cubic
	bool extFilterCubicEnabled = false;
	// VK_EXT_global_priority
	bool extGlobalPriorityEnabled = false;
	// VK_KHR_shader_subgroup_extended_types
	bool khrShaderSubgroupExtendedTypesEnabled = false;
	// VK_KHR_8bit_storage
	bool khr8BitStorageEnabled = false;
	// VK_EXT_external_memory_host
	bool extExternalMemoryHostEnabled = false;
	// VK_AMD_buffer_marker
	bool amdBufferMarkerEnabled = false;
	// VK_KHR_shader_atomic_int64
	bool khrShaderAtomicInt64Enabled = false;
	// VK_KHR_shader_clock
	bool khrShaderClockEnabled = false;
	// VK_AMD_pipeline_compiler_control
	bool amdPipelineCompilerControlEnabled = false;
	// VK_EXT_calibrated_timestamps
	bool extCalibratedTimestampsEnabled = false;
	// VK_AMD_shader_core_properties
	bool amdShaderCorePropertiesEnabled = false;
	// VK_AMD_memory_overallocation_behavior
	bool amdMemoryOverallocationBehaviorEnabled = false;
	// VK_EXT_vertex_attribute_divisor
	bool extVertexAttributeDivisorEnabled = false;
	// VK_GGP_frame_token
	bool ggpFrameTokenEnabled = false;
	// VK_EXT_pipeline_creation_feedback
	bool extPipelineCreationFeedbackEnabled = false;
	// VK_KHR_driver_properties
	bool khrDriverPropertiesEnabled = false;
	// VK_KHR_shader_float_controls
	bool khrShaderFloatControlsEnabled = false;
	// VK_NV_shader_subgroup_partitioned
	bool nvShaderSubgroupPartitionedEnabled = false;
	// VK_KHR_depth_stencil_resolve
	bool khrDepthStencilResolveEnabled = false;
	// VK_KHR_swapchain_mutable_format
	bool khrSwapchainMutableFormatEnabled = false;
	// VK_NV_compute_shader_derivatives
	bool nvComputeShaderDerivativesEnabled = false;
	// VK_NV_mesh_shader
	bool nvMeshShaderEnabled = false;
	// VK_NV_fragment_shader_barycentric
	bool nvFragmentShaderBarycentricEnabled = false;
	// VK_NV_shader_image_footprint
	bool nvShaderImageFootprintEnabled = false;
	// VK_NV_scissor_exclusive
	bool nvScissorExclusiveEnabled = false;
	// VK_NV_device_diagnostic_checkpoints
	bool nvDeviceDiagnosticCheckpointsEnabled = false;
	// VK_KHR_timeline_semaphore
	bool khrTimelineSemaphoreEnabled = false;
	// VK_INTEL_shader_integer_functions2
	bool intelShaderIntegerFunctions2Enabled = false;
	// VK_INTEL_performance_query
	bool intelPerformanceQueryEnabled = false;
	// VK_KHR_vulkan_memory_model
	bool khrVulkanMemoryModelEnabled = false;
	// VK_EXT_pci_bus_info
	bool extPciBusInfoEnabled = false;
	// VK_AMD_display_native_hdr
	bool amdDisplayNativeHdrEnabled = false;
	// VK_EXT_fragment_density_map
	bool extFragmentDensityMapEnabled = false;
	// VK_EXT_scalar_block_layout
	bool extScalarBlockLayoutEnabled = false;
	// VK_GOOGLE_hlsl_functionality1
	bool googleHlslFunctionality1Enabled = false;
	// VK_GOOGLE_decorate_string
	bool googleDecorateStringEnabled = false;
	// VK_EXT_subgroup_size_control
	bool extSubgroupSizeControlEnabled = false;
	// VK_AMD_shader_core_properties2
	bool amdShaderCoreProperties2Enabled = false;
	// VK_AMD_device_coherent_memory
	bool amdDeviceCoherentMemoryEnabled = false;
	// VK_KHR_spirv_1_4
	bool khrSpirv14Enabled = false;
	// VK_EXT_memory_budget
	bool extMemoryBudgetEnabled = false;
	// VK_EXT_memory_priority
	bool extMemoryPriorityEnabled = false;
	// VK_NV_dedicated_allocation_image_aliasing
	bool nvDedicatedAllocationImageAliasingEnabled = false;
	// VK_KHR_separate_depth_stencil_layouts
	bool khrSeparateDepthStencilLayoutsEnabled = false;
	// VK_EXT_buffer_device_address
	bool extBufferDeviceAddressEnabled = false;
	// VK_EXT_tooling_info
	bool extToolingInfoEnabled = false;
	// VK_EXT_separate_stencil_usage
	bool extSeparateStencilUsageEnabled = false;
	// VK_NV_cooperative_matrix
	bool nvCooperativeMatrixEnabled = false;
	// VK_NV_coverage_reduction_mode
	bool nvCoverageReductionModeEnabled = false;
	// VK_EXT_fragment_shader_interlock
	bool extFragmentShaderInterlockEnabled = false;
	// VK_EXT_ycbcr_image_arrays
	bool extYcbcrImageArraysEnabled = false;
	// VK_KHR_uniform_buffer_standard_layout
	bool khrUniformBufferStandardLayoutEnabled = false;
	// VK_EXT_full_screen_exclusive
	bool extFullScreenExclusiveEnabled = false;
	// VK_KHR_buffer_device_address
	bool khrBufferDeviceAddressEnabled = false;
	// VK_EXT_line_rasterization
	bool extLineRasterizationEnabled = false;
	// VK_EXT_host_query_reset
	bool extHostQueryResetEnabled = false;
	// VK_EXT_index_type_uint8
	bool extIndexTypeUint8Enabled = false;
	// VK_KHR_pipeline_executable_properties
	bool khrPipelineExecutablePropertiesEnabled = false;
	// VK_EXT_shader_demote_to_helper_invocation
	bool extShaderDemoteToHelperInvocationEnabled = false;
	// VK_EXT_texel_buffer_alignment
	bool extTexelBufferAlignmentEnabled = false;
	// VK_GOOGLE_user_type
	bool googleUserTypeEnabled = false;

void setEnabledExtension(const std::string& extension)
{
	std::string deviceExtensions[162] = {
		"VK_KHR_swapchain",
		"VK_KHR_display_swapchain",
		"VK_NV_glsl_shader",
		"VK_EXT_depth_range_unrestricted",
		"VK_KHR_sampler_mirror_clamp_to_edge",
		"VK_IMG_filter_cubic",
		"VK_AMD_rasterization_order",
		"VK_AMD_shader_trinary_minmax",
		"VK_AMD_shader_explicit_vertex_parameter",
		"VK_EXT_debug_marker",
		"VK_AMD_gcn_shader",
		"VK_NV_dedicated_allocation",
		"VK_EXT_transform_feedback",
		"VK_NVX_image_view_handle",
		"VK_AMD_draw_indirect_count",
		"VK_AMD_negative_viewport_height",
		"VK_AMD_gpu_shader_half_float",
		"VK_AMD_shader_ballot",
		"VK_AMD_texture_gather_bias_lod",
		"VK_AMD_shader_info",
		"VK_AMD_shader_image_load_store_lod",
		"VK_NV_corner_sampled_image",
		"VK_KHR_multiview",
		"VK_IMG_format_pvrtc",
		"VK_NV_external_memory",
		"VK_NV_external_memory_win32",
		"VK_NV_win32_keyed_mutex",
		"VK_KHR_device_group",
		"VK_KHR_shader_draw_parameters",
		"VK_EXT_shader_subgroup_ballot",
		"VK_EXT_shader_subgroup_vote",
		"VK_EXT_texture_compression_astc_hdr",
		"VK_EXT_astc_decode_mode",
		"VK_KHR_maintenance1",
		"VK_KHR_external_memory",
		"VK_KHR_external_memory_win32",
		"VK_KHR_external_memory_fd",
		"VK_KHR_win32_keyed_mutex",
		"VK_KHR_external_semaphore",
		"VK_KHR_external_semaphore_win32",
		"VK_KHR_external_semaphore_fd",
		"VK_KHR_push_descriptor",
		"VK_EXT_conditional_rendering",
		"VK_KHR_shader_float16_int8",
		"VK_KHR_16bit_storage",
		"VK_KHR_incremental_present",
		"VK_KHR_descriptor_update_template",
		"VK_NVX_device_generated_commands",
		"VK_NV_clip_space_w_scaling",
		"VK_EXT_display_control",
		"VK_GOOGLE_display_timing",
		"VK_NV_sample_mask_override_coverage",
		"VK_NV_geometry_shader_passthrough",
		"VK_NV_viewport_array2",
		"VK_NVX_multiview_per_view_attributes",
		"VK_NV_viewport_swizzle",
		"VK_EXT_discard_rectangles",
		"VK_EXT_conservative_rasterization",
		"VK_EXT_depth_clip_enable",
		"VK_EXT_hdr_metadata",
		"VK_KHR_imageless_framebuffer",
		"VK_KHR_create_renderpass2",
		"VK_KHR_shared_presentable_image",
		"VK_KHR_external_fence",
		"VK_KHR_external_fence_win32",
		"VK_KHR_external_fence_fd",
		"VK_KHR_performance_query",
		"VK_KHR_maintenance2",
		"VK_KHR_variable_pointers",
		"VK_EXT_external_memory_dma_buf",
		"VK_EXT_queue_family_foreign",
		"VK_KHR_dedicated_allocation",
		"VK_ANDROID_external_memory_android_hardware_buffer",
		"VK_EXT_sampler_filter_minmax",
		"VK_KHR_storage_buffer_storage_class",
		"VK_AMD_gpu_shader_int16",
		"VK_AMD_mixed_attachment_samples",
		"VK_AMD_shader_fragment_mask",
		"VK_EXT_inline_uniform_block",
		"VK_EXT_shader_stencil_export",
		"VK_EXT_sample_locations",
		"VK_KHR_relaxed_block_layout",
		"VK_KHR_get_memory_requirements2",
		"VK_KHR_image_format_list",
		"VK_EXT_blend_operation_advanced",
		"VK_NV_fragment_coverage_to_color",
		"VK_NV_framebuffer_mixed_samples",
		"VK_NV_fill_rectangle",
		"VK_NV_shader_sm_builtins",
		"VK_EXT_post_depth_coverage",
		"VK_KHR_sampler_ycbcr_conversion",
		"VK_KHR_bind_memory2",
		"VK_EXT_image_drm_format_modifier",
		"VK_EXT_validation_cache",
		"VK_EXT_descriptor_indexing",
		"VK_EXT_shader_viewport_index_layer",
		"VK_NV_shading_rate_image",
		"VK_NV_ray_tracing",
		"VK_NV_representative_fragment_test",
		"VK_KHR_maintenance3",
		"VK_KHR_draw_indirect_count",
		"VK_EXT_filter_cubic",
		"VK_EXT_global_priority",
		"VK_KHR_shader_subgroup_extended_types",
		"VK_KHR_8bit_storage",
		"VK_EXT_external_memory_host",
		"VK_AMD_buffer_marker",
		"VK_KHR_shader_atomic_int64",
		"VK_KHR_shader_clock",
		"VK_AMD_pipeline_compiler_control",
		"VK_EXT_calibrated_timestamps",
		"VK_AMD_shader_core_properties",
		"VK_AMD_memory_overallocation_behavior",
		"VK_EXT_vertex_attribute_divisor",
		"VK_GGP_frame_token",
		"VK_EXT_pipeline_creation_feedback",
		"VK_KHR_driver_properties",
		"VK_KHR_shader_float_controls",
		"VK_NV_shader_subgroup_partitioned",
		"VK_KHR_depth_stencil_resolve",
		"VK_KHR_swapchain_mutable_format",
		"VK_NV_compute_shader_derivatives",
		"VK_NV_mesh_shader",
		"VK_NV_fragment_shader_barycentric",
		"VK_NV_shader_image_footprint",
		"VK_NV_scissor_exclusive",
		"VK_NV_device_diagnostic_checkpoints",
		"VK_KHR_timeline_semaphore",
		"VK_INTEL_shader_integer_functions2",
		"VK_INTEL_performance_query",
		"VK_KHR_vulkan_memory_model",
		"VK_EXT_pci_bus_info",
		"VK_AMD_display_native_hdr",
		"VK_EXT_fragment_density_map",
		"VK_EXT_scalar_block_layout",
		"VK_GOOGLE_hlsl_functionality1",
		"VK_GOOGLE_decorate_string",
		"VK_EXT_subgroup_size_control",
		"VK_AMD_shader_core_properties2",
		"VK_AMD_device_coherent_memory",
		"VK_KHR_spirv_1_4",
		"VK_EXT_memory_budget",
		"VK_EXT_memory_priority",
		"VK_NV_dedicated_allocation_image_aliasing",
		"VK_KHR_separate_depth_stencil_layouts",
		"VK_EXT_buffer_device_address",
		"VK_EXT_tooling_info",
		"VK_EXT_separate_stencil_usage",
		"VK_NV_cooperative_matrix",
		"VK_NV_coverage_reduction_mode",
		"VK_EXT_fragment_shader_interlock",
		"VK_EXT_ycbcr_image_arrays",
		"VK_KHR_uniform_buffer_standard_layout",
		"VK_EXT_full_screen_exclusive",
		"VK_KHR_buffer_device_address",
		"VK_EXT_line_rasterization",
		"VK_EXT_host_query_reset",
		"VK_EXT_index_type_uint8",
		"VK_KHR_pipeline_executable_properties",
		"VK_EXT_shader_demote_to_helper_invocation",
		"VK_EXT_texel_buffer_alignment",
		"VK_GOOGLE_user_type",
	};

	bool* deviceExtensionsFlags[162] = {
		&khrSwapchainEnabled,
		&khrDisplaySwapchainEnabled,
		&nvGlslShaderEnabled,
		&extDepthRangeUnrestrictedEnabled,
		&khrSamplerMirrorClampToEdgeEnabled,
		&imgFilterCubicEnabled,
		&amdRasterizationOrderEnabled,
		&amdShaderTrinaryMinmaxEnabled,
		&amdShaderExplicitVertexParameterEnabled,
		&extDebugMarkerEnabled,
		&amdGcnShaderEnabled,
		&nvDedicatedAllocationEnabled,
		&extTransformFeedbackEnabled,
		&nvxImageViewHandleEnabled,
		&amdDrawIndirectCountEnabled,
		&amdNegativeViewportHeightEnabled,
		&amdGpuShaderHalfFloatEnabled,
		&amdShaderBallotEnabled,
		&amdTextureGatherBiasLodEnabled,
		&amdShaderInfoEnabled,
		&amdShaderImageLoadStoreLodEnabled,
		&nvCornerSampledImageEnabled,
		&khrMultiviewEnabled,
		&imgFormatPvrtcEnabled,
		&nvExternalMemoryEnabled,
		&nvExternalMemoryWin32Enabled,
		&nvWin32KeyedMutexEnabled,
		&khrDeviceGroupEnabled,
		&khrShaderDrawParametersEnabled,
		&extShaderSubgroupBallotEnabled,
		&extShaderSubgroupVoteEnabled,
		&extTextureCompressionAstcHdrEnabled,
		&extAstcDecodeModeEnabled,
		&khrMaintenance1Enabled,
		&khrExternalMemoryEnabled,
		&khrExternalMemoryWin32Enabled,
		&khrExternalMemoryFdEnabled,
		&khrWin32KeyedMutexEnabled,
		&khrExternalSemaphoreEnabled,
		&khrExternalSemaphoreWin32Enabled,
		&khrExternalSemaphoreFdEnabled,
		&khrPushDescriptorEnabled,
		&extConditionalRenderingEnabled,
		&khrShaderFloat16Int8Enabled,
		&khr16BitStorageEnabled,
		&khrIncrementalPresentEnabled,
		&khrDescriptorUpdateTemplateEnabled,
		&nvxDeviceGeneratedCommandsEnabled,
		&nvClipSpaceWScalingEnabled,
		&extDisplayControlEnabled,
		&googleDisplayTimingEnabled,
		&nvSampleMaskOverrideCoverageEnabled,
		&nvGeometryShaderPassthroughEnabled,
		&nvViewportArray2Enabled,
		&nvxMultiviewPerViewAttributesEnabled,
		&nvViewportSwizzleEnabled,
		&extDiscardRectanglesEnabled,
		&extConservativeRasterizationEnabled,
		&extDepthClipEnableEnabled,
		&extHdrMetadataEnabled,
		&khrImagelessFramebufferEnabled,
		&khrCreateRenderpass2Enabled,
		&khrSharedPresentableImageEnabled,
		&khrExternalFenceEnabled,
		&khrExternalFenceWin32Enabled,
		&khrExternalFenceFdEnabled,
		&khrPerformanceQueryEnabled,
		&khrMaintenance2Enabled,
		&khrVariablePointersEnabled,
		&extExternalMemoryDmaBufEnabled,
		&extQueueFamilyForeignEnabled,
		&khrDedicatedAllocationEnabled,
		&androidExternalMemoryAndroidHardwareBufferEnabled,
		&extSamplerFilterMinmaxEnabled,
		&khrStorageBufferStorageClassEnabled,
		&amdGpuShaderInt16Enabled,
		&amdMixedAttachmentSamplesEnabled,
		&amdShaderFragmentMaskEnabled,
		&extInlineUniformBlockEnabled,
		&extShaderStencilExportEnabled,
		&extSampleLocationsEnabled,
		&khrRelaxedBlockLayoutEnabled,
		&khrGetMemoryRequirements2Enabled,
		&khrImageFormatListEnabled,
		&extBlendOperationAdvancedEnabled,
		&nvFragmentCoverageToColorEnabled,
		&nvFramebufferMixedSamplesEnabled,
		&nvFillRectangleEnabled,
		&nvShaderSmBuiltinsEnabled,
		&extPostDepthCoverageEnabled,
		&khrSamplerYcbcrConversionEnabled,
		&khrBindMemory2Enabled,
		&extImageDrmFormatModifierEnabled,
		&extValidationCacheEnabled,
		&extDescriptorIndexingEnabled,
		&extShaderViewportIndexLayerEnabled,
		&nvShadingRateImageEnabled,
		&nvRayTracingEnabled,
		&nvRepresentativeFragmentTestEnabled,
		&khrMaintenance3Enabled,
		&khrDrawIndirectCountEnabled,
		&extFilterCubicEnabled,
		&extGlobalPriorityEnabled,
		&khrShaderSubgroupExtendedTypesEnabled,
		&khr8BitStorageEnabled,
		&extExternalMemoryHostEnabled,
		&amdBufferMarkerEnabled,
		&khrShaderAtomicInt64Enabled,
		&khrShaderClockEnabled,
		&amdPipelineCompilerControlEnabled,
		&extCalibratedTimestampsEnabled,
		&amdShaderCorePropertiesEnabled,
		&amdMemoryOverallocationBehaviorEnabled,
		&extVertexAttributeDivisorEnabled,
		&ggpFrameTokenEnabled,
		&extPipelineCreationFeedbackEnabled,
		&khrDriverPropertiesEnabled,
		&khrShaderFloatControlsEnabled,
		&nvShaderSubgroupPartitionedEnabled,
		&khrDepthStencilResolveEnabled,
		&khrSwapchainMutableFormatEnabled,
		&nvComputeShaderDerivativesEnabled,
		&nvMeshShaderEnabled,
		&nvFragmentShaderBarycentricEnabled,
		&nvShaderImageFootprintEnabled,
		&nvScissorExclusiveEnabled,
		&nvDeviceDiagnosticCheckpointsEnabled,
		&khrTimelineSemaphoreEnabled,
		&intelShaderIntegerFunctions2Enabled,
		&intelPerformanceQueryEnabled,
		&khrVulkanMemoryModelEnabled,
		&extPciBusInfoEnabled,
		&amdDisplayNativeHdrEnabled,
		&extFragmentDensityMapEnabled,
		&extScalarBlockLayoutEnabled,
		&googleHlslFunctionality1Enabled,
		&googleDecorateStringEnabled,
		&extSubgroupSizeControlEnabled,
		&amdShaderCoreProperties2Enabled,
		&amdDeviceCoherentMemoryEnabled,
		&khrSpirv14Enabled,
		&extMemoryBudgetEnabled,
		&extMemoryPriorityEnabled,
		&nvDedicatedAllocationImageAliasingEnabled,
		&khrSeparateDepthStencilLayoutsEnabled,
		&extBufferDeviceAddressEnabled,
		&extToolingInfoEnabled,
		&extSeparateStencilUsageEnabled,
		&nvCooperativeMatrixEnabled,
		&nvCoverageReductionModeEnabled,
		&extFragmentShaderInterlockEnabled,
		&extYcbcrImageArraysEnabled,
		&khrUniformBufferStandardLayoutEnabled,
		&extFullScreenExclusiveEnabled,
		&khrBufferDeviceAddressEnabled,
		&extLineRasterizationEnabled,
		&extHostQueryResetEnabled,
		&extIndexTypeUint8Enabled,
		&khrPipelineExecutablePropertiesEnabled,
		&extShaderDemoteToHelperInvocationEnabled,
		&extTexelBufferAlignmentEnabled,
		&googleUserTypeEnabled,
	};

	for(uint32_t i = 0; i < 162; ++i)
	{
		if (extension == deviceExtensions[i])
		{
			*deviceExtensionsFlags[i] = true;
		}
	}
}

void setEnabledExtensions(const std::vector<std::string>& extensions)
{
	for(uint32_t i = 0; i < static_cast<uint32_t>(extensions.size()); ++i)
	{
		setEnabledExtension(extensions[i]);
	}
}

void setEnabledExtensions(const std::vector<const char*>& extensions)
{
	for(uint32_t i = 0; i < static_cast<uint32_t>(extensions.size()); ++i)
	{
		setEnabledExtension(extensions[i]);
	}
}
};

} // namespace pvrvk
// clang-format on
//!\endcond
