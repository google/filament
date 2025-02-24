/*!
\brief Forward declarations of all pvrvk:: objects
\file PVRVk/ForwardDecObjectsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/HeadersVk.h"
#include "PVRVk/TypesVk.h"
#include <array>
namespace pvrvk {
//!\cond NO_DOXYGEN
namespace impl {
class Device_;
class Image_;
class SwapchainImage_;
class Framebuffer_;
class Buffer_;
class BufferView_;
class Sampler_;
class ImageView_;
class ShaderModule_;
class RenderPass_;
class DescriptorSet_;
class DescriptorSetLayout_;
class DescriptorPool_;
class CommandBufferBase_;
class CommandBuffer_;
class SecondaryCommandBuffer_;
class Pipeline_;
class GraphicsPipeline_;
class ComputePipeline_;
class PipelineLayout_;
class PipelineCache_;
class CommandPool_;
class Fence_;
class Semaphore_;
class Event_;
class IDeviceMemory_;
class DeviceMemory_;
class Swapchain_;
class PhysicalDevice_;
class Display_;
class DisplayMode_;
class Surface_;
class AndroidSurface_;
class Win32Surface_;
class XcbSurface_;
class XlibSurface_;
class WaylandSurface_;
class MacOSSurface_;
class DisplayPlaneSurface_;
class Queue_;
class PipelineCache_;
class Instance_;
class QueryPool_;
class DebugReportCallback_;
class DebugUtilsMessenger_;
} // namespace impl
//!\endcond

struct MemoryBarrierSet;
struct GraphicsPipelineCreateInfo;
struct RenderPassCreateInfo;
struct ComputePipelineCreateInfo;
struct FramebufferCreateInfo;
struct DescriptorSetLayoutCreateInfo;
struct DescriptorPoolCreateInfo;
struct WriteDescriptorSet;
struct CopyDescriptorSet;
struct PipelineLayoutCreateInfo;
struct SwapchainCreateInfo;
struct DeviceQueueCreateInfo;
struct DebugReportCallbackCreateInfo;
struct DebugUtilsMessengerCreateInfo;
struct DisplayModeCreateInfo;
struct ImageCreateInfo;
struct BufferCreateInfo;
struct ImageViewCreateInfo;
struct BufferViewCreateInfo;
struct PipelineCacheCreateInfo;
struct CommandPoolCreateInfo;
struct FenceCreateInfo;
struct EventCreateInfo;
struct SemaphoreCreateInfo;
struct QueryPoolCreateInfo;
struct ShaderModuleCreateInfo;
struct MemoryAllocationInfo;
struct ExportMemoryAllocateInfoKHR;

/// <summary>Forwared-declared reference-counted handle to a Framebuffer. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Framebuffer_> Framebuffer;

/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Buffer_> Buffer;

/// <summary>Forwared-declared reference-counted handle to a GraphicsPipeline. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::GraphicsPipeline_> GraphicsPipeline;

/// <summary>Forwared-declared reference-counted handle to a ComputePipeline. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::ComputePipeline_> ComputePipeline;

/// <summary>Forwared-declared reference-counted handle to a Sampler. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Sampler_> Sampler;

/// <summary>A generic Buffer. Can be directly bound as a VBO /IBO or wrapped with a BufferView(SsboView,
/// UboView) to be bound via a DescriptorSet</summary>
typedef std::shared_ptr<impl::BufferView_> BufferView;

/// <summary>DeviceMemory.</summary>
typedef std::shared_ptr<impl::IDeviceMemory_> DeviceMemory;

/// <summary>DeviceMemory.</summary>
typedef std::unique_ptr<impl::DeviceMemory_> DeviceMemoryImpl;

/// <summary>A ShaderModule object.</summary>
typedef std::shared_ptr<impl::ShaderModule_> ShaderModule;

/// <summary>An RenderPass object represents a drawing cycle that ends up rendering to a single Framebuffer.</summary>
typedef std::shared_ptr<impl::RenderPass_> RenderPass;

/// <summary>A DescriptorSet represents a collection of resources (Textures, Buffers, Samplers, etc.) that can
/// all be bound together for use by a rendering run.</summary>
typedef std::shared_ptr<impl::DescriptorSet_> DescriptorSet;

/// <summary>A DescriptorSet Layout represents a "recipe" for a descriptor set. It is used for other objects to
/// ensure compatibility with a specific DescriptorSet family.</summary>
typedef std::shared_ptr<impl::DescriptorSetLayout_> DescriptorSetLayout;

/// <summary>DescriptorSetLayout array type</summary>
typedef std::array<DescriptorSetLayout, FrameworkCaps::MaxDescriptorSetBindings> DescriptorSetLayoutSet;

/// <summary>A backing store for any kind of texture.</summary>
typedef std::shared_ptr<impl::Image_> Image;

/// <summary>An PipelineCache object</summary>
typedef std::shared_ptr<impl::PipelineCache_> PipelineCache;

/// <summary>A dummy backing store for a swapchain image.</summary>
typedef std::shared_ptr<impl::SwapchainImage_> SwapchainImage;

/// <summary>Base class for the view of any kind of texture view.</summary>
typedef std::shared_ptr<impl::ImageView_> ImageView;

/// <summary>A descriptor pool represents a specific chunk of memory from which descriptor pools will be
/// allocated. It is intended that different threads will use different descriptor pools to avoid having contention
/// and the need to lock between them.</summary>
typedef std::shared_ptr<impl::DescriptorPool_> DescriptorPool;

/// <summary>A CommandBuffer(Base) represents a std::string of commands that will be submitted to the GPU in a batch.</summary>
typedef std::shared_ptr<impl::CommandBufferBase_> CommandBufferBase;

/// <summary>A CommandBuffer(Primary) is a CommandBuffer that can be submitted to the GPU and can contain
/// secondary command buffers</summary>
typedef std::shared_ptr<impl::CommandBuffer_> CommandBuffer;

/// <summary>A SecondaryCommandBufferis a CommandBuffer that can only be submitted to a primary CommandBuffer
/// and cannot contain a RenderPass</summary>
typedef std::shared_ptr<impl::SecondaryCommandBuffer_> SecondaryCommandBuffer;

/// <summary>A PipelineLayout represents the blueprint out of which a pipeline will be created, needed by other
/// objects to ensure compatibility with a family of GraphicsPipelines.</summary>
typedef std::shared_ptr<impl::PipelineLayout_> PipelineLayout;

/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Buffer_> Buffer;

/// <summary>Forwared-declared reference-counted handle to a CommandPool. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::CommandPool_> CommandPool;

/// <summary>Forwared-declared weak-reference-counted handle to a CommandPool. For detailed documentation, see PVRVk module</summary>
typedef std::weak_ptr<impl::CommandPool_> CommandPoolWeakPtr;

/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Fence_> Fence;

/// <summary>Forwared-declared reference-counted handle to a Swapchain. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Swapchain_> Swapchain;

/// <summary>Forwared-declared reference-counted handle to a Surface. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Surface_> Surface;

/// <summary>Forwared-declared reference-counted handle to an AndroidSurface. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::AndroidSurface_> AndroidSurface;

/// <summary>Forwared-declared reference-counted handle to a Win32Surface. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Win32Surface_> Win32Surface;

/// <summary>Forwared-declared reference-counted handle to a XcbSurface. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::XcbSurface_> XcbSurface;

/// <summary>Forwared-declared reference-counted handle to a XlibSurface. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::XlibSurface_> XlibSurface;

/// <summary>Forwared-declared reference-counted handle to a WaylandSurface. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::WaylandSurface_> WaylandSurface;

/// <summary>Forwared-declared reference-counted handle to a MacOSSurface. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::MacOSSurface_> MacOSSurface;

/// <summary>Forwared-declared reference-counted handle to a DisplayPlaneSurface. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::DisplayPlaneSurface_> DisplayPlaneSurface;

/// <summary>Forwared-declared weak-reference-counted handle to a Surface. For detailed documentation, see PVRVk module</summary>
typedef std::weak_ptr<impl::Surface_> SurfaceWeakPtr;

/// <summary>Forwared-declared reference-counted handle to a PhyscialDevice. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::PhysicalDevice_> PhysicalDevice;

/// <summary>Forwared-declared weak-reference-counted handle to a PhysicalDevice. For detailed documentation, see PVRVk module</summary>
typedef std::weak_ptr<impl::PhysicalDevice_> PhysicalDeviceWeakPtr;

/// <summary>Forwared-declared reference-counted handle to a Display. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Display_> Display;

/// <summary>Forwared-declared reference-counted handle to a DisplayMode. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::DisplayMode_> DisplayMode;

/// <summary>Forwared-declared weak-reference-counted handle to a DisplayMode. For detailed documentation, see PVRVk module</summary>
typedef std::weak_ptr<impl::DisplayMode_> DisplayModeWeakPtr;

/// <summary>Forwared-declared reference-counted handle to a Instance. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Instance_> Instance;

/// <summary>Forwared-declared weak-reference-counted handle to a Instance. For detailed documentation, see PVRVk module</summary>
typedef std::weak_ptr<impl::Instance_> InstanceWeakPtr;

/// <summary>Forwared-declared reference-counted handle to a Queue. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Queue_> Queue;

/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Semaphore_> Semaphore;

/// <summary>Forwared-declared reference-counted handle to a Device. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Device_> Device;

/// <summary>Forwared-declared weak-reference-counted handle to a Device. For detailed documentation, see PVRVk module</summary>
typedef std::weak_ptr<impl::Device_> DeviceWeakPtr;

/// <summary>Forwared-declared reference-counted handle to a Buffer. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::Event_> Event;

/// <summary>Forwared-declared reference-counted handle to a QueryPool. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::QueryPool_> QueryPool;

/// <summary>Forwared-declared reference-counted handle to a DebugReportCallback. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::DebugReportCallback_> DebugReportCallback;

/// <summary>Forwared-declared reference-counted handle to a DebugUtilsMessenger. For detailed documentation, see PVRVk module</summary>
typedef std::shared_ptr<impl::DebugUtilsMessenger_> DebugUtilsMessenger;
} // namespace pvrvk
