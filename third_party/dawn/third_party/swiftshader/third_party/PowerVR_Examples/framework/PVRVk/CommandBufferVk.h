/*!
\brief The CommandBuffer class, arguably the busiest class in Vulkan, containing most functionality.
\file PVRVk/CommandBufferVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/GraphicsPipelineVk.h"
#include "PVRVk/ComputePipelineVk.h"
#include "PVRVk/EventVk.h"
#include "PVRVk/FramebufferVk.h"
#include "PVRVk/RenderPassVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Contains all the commands and states that need to be recorded for later submission to the gpu including pipelines,
/// textures, descriptor sets. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer.</summary>
class CommandBufferBase_ : public PVRVkDeviceObjectBase<VkCommandBuffer, ObjectType::e_COMMAND_BUFFER>, public DeviceObjectDebugUtils<CommandBufferBase_>
{
protected:
	/// <summary>A class which restricts the creation of a pvrvk::CommandBufferBase to children or friends of a pvrvk::impl::CommandBufferBase_.</summary>
	class make_shared_enabler
	{
	protected:
		/// <summary>Constructor for a make_shared_enabler.</summary>
		make_shared_enabler() = default;
		friend class CommandBufferBase_;
	};

	/// <summary>Protected function used to create a pvrvk::CommandBufferBase. Note that this function shouldn't normally be called
	/// directly and will be called by a friend of CommandBufferBase_ which will generally be a CommandPool.</summary>
	/// <param name="device">The device used to allocate the command buffer.</param>
	/// <param name="pool">The pool from which the command buffer will be allocated.</param>
	/// <param name="myHandle">The vulkan handle for this command buffer.</param>
	/// <returns>Returns a successfully created pvrvk::CommandBufferBase</returns>
	static CommandBufferBase constructShared(const DeviceWeakPtr& device, CommandPool& pool, VkCommandBuffer myHandle)
	{
		return std::make_shared<CommandBufferBase_>(make_shared_enabler{}, device, pool, myHandle);
	}

	/// <summary>Holds a list of references to the objects currently in use by this command buffer. This ensures that objects are kept alive through
	/// reference counting until the command buffer is finished with them.</summary>
	std::vector<std::shared_ptr<void> /**/> _objectReferences;

	/// <summary>The command pool from which this command buffer was allocated.</summary>
	CommandPool _pool;

	/// <summary>Specifies whether the command buffer is currently in the recording state which is controlled via calling the begin function.</summary>
	bool _isRecording;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(CommandBufferBase_)
	/// <summary>Constructor. This constructor shouldn't be called directly and should instead be called indirectly via a call to
	/// CommandPool::allocateCommandBuffers.</summary>
	/// <param name="device">The device used to allocate this command buffer.</param>
	/// <param name="pool">The pool from which the command buffer was allocated.</param>
	/// <param name="myHandle">The vulkan handle for this command buffer.</param>
	CommandBufferBase_(make_shared_enabler, const DeviceWeakPtr& device, CommandPool pool, VkCommandBuffer myHandle)
		: PVRVkDeviceObjectBase(device, myHandle), DeviceObjectDebugUtils(), _pool(pool), _isRecording(false)
	{}

	/// <summary>Destructor. Virtual (for polymorphic use).</summary>
	virtual ~CommandBufferBase_();
	//!\endcond

	/// <summary>Call this function before beginning to record commands.</summary>
	/// <param name="flags">Flags is a bitmask of CommandBufferUsageFlags specifying usage behavior for the command buffer.</param>
	void begin(const CommandBufferUsageFlags flags = CommandBufferUsageFlags(0));

	/// <summary>Call this function when you are done recording commands. BeginRecording must be called first.</summary>
	void end();

	/// <summary>Begins identifying a region of work submitted to this command buffer. The calls to beginDebugUtilsLabel and endDebugUtilsLabel must be matched and
	/// balanced.</summary>
	/// <param name="labelInfo">Specifies the parameters of the label region to open</param>
	void beginDebugUtilsLabel(const pvrvk::DebugUtilsLabel& labelInfo)
	{
		VkDebugUtilsLabelEXT vkLabelInfo = {};
		vkLabelInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_LABEL_EXT);
		// The color to use for the marked region
		vkLabelInfo.color[0] = labelInfo.getR();
		vkLabelInfo.color[1] = labelInfo.getG();
		vkLabelInfo.color[2] = labelInfo.getB();
		vkLabelInfo.color[3] = labelInfo.getA();
		// The label name to give to the marked region
		vkLabelInfo.pLabelName = labelInfo.getLabelName().c_str();
		getDevice()->getPhysicalDevice()->getInstance()->getVkBindings().vkCmdBeginDebugUtilsLabelEXT(getVkHandle(), &vkLabelInfo);
	}

	/// <summary>Ends a label region of work submitted to this command buffer.</summary>
	void endDebugUtilsLabel() { getDevice()->getPhysicalDevice()->getInstance()->getVkBindings().vkCmdEndDebugUtilsLabelEXT(getVkHandle()); }

	/// <summary>Inserts a single debug label any time.</summary>
	/// <param name="labelInfo">Specifies the parameters of the label region to insert</param>
	void insertDebugUtilsLabel(const pvrvk::DebugUtilsLabel& labelInfo)
	{
		VkDebugUtilsLabelEXT vkLabelInfo = {};
		vkLabelInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_LABEL_EXT);
		// The color to use for the marked region
		vkLabelInfo.color[0] = labelInfo.getR();
		vkLabelInfo.color[1] = labelInfo.getG();
		vkLabelInfo.color[2] = labelInfo.getB();
		vkLabelInfo.color[3] = labelInfo.getA();
		// The label name to give to the marked region
		vkLabelInfo.pLabelName = labelInfo.getLabelName().c_str();
		getDevice()->getPhysicalDevice()->getInstance()->getVkBindings().vkCmdInsertDebugUtilsLabelEXT(getVkHandle(), &vkLabelInfo);
	}

	/// <summary>Begins a debug marked region.</summary>
	/// <param name="markerInfo">Specifies the creation info for a marked region.</param>
	void debugMarkerBeginEXT(pvrvk::DebugMarkerMarkerInfo& markerInfo)
	{
		VkDebugMarkerMarkerInfoEXT vkMarkerInfo = {};
		vkMarkerInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_MARKER_MARKER_INFO_EXT);
		// The color to use for the marked region
		vkMarkerInfo.color[0] = markerInfo.getR();
		vkMarkerInfo.color[1] = markerInfo.getG();
		vkMarkerInfo.color[2] = markerInfo.getB();
		vkMarkerInfo.color[3] = markerInfo.getA();
		// The name to give to the marked region
		vkMarkerInfo.pMarkerName = markerInfo.getMarkerName().c_str();
		getDevice()->getVkBindings().vkCmdDebugMarkerBeginEXT(getVkHandle(), &vkMarkerInfo);
	}

	/// <summary>Ends a debug marked region.</summary>
	void debugMarkerEndEXT() { getDevice()->getVkBindings().vkCmdDebugMarkerEndEXT(getVkHandle()); }

	/// <summary>Inserts a debug marker.</summary>
	/// <param name="markerInfo">Specifies creation info for the marker.</param>
	void debugMarkerInsertEXT(pvrvk::DebugMarkerMarkerInfo& markerInfo)
	{
		VkDebugMarkerMarkerInfoEXT vkMarkerInfo = {};
		vkMarkerInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_MARKER_MARKER_INFO_EXT);
		// The color to use for the marked region
		vkMarkerInfo.color[0] = markerInfo.getR();
		vkMarkerInfo.color[1] = markerInfo.getG();
		vkMarkerInfo.color[2] = markerInfo.getB();
		vkMarkerInfo.color[3] = markerInfo.getA();
		// The name to give to the marked region
		vkMarkerInfo.pMarkerName = markerInfo.getMarkerName().c_str();
		getDevice()->getVkBindings().vkCmdDebugMarkerInsertEXT(getVkHandle(), &vkMarkerInfo);
	}

	/// <summary>Resets a particular range of queries for a particular QueryPool and sets their status' to unavailable which also makes their numerical results undefined.</summary>
	/// <param name="queryPool">Specifies the query pool managing the queries being reset.</param>
	/// <param name="firstQuery">The first query index to reset.</param>
	/// <param name="queryCount">The number of queries to reset.</param>
	void resetQueryPool(QueryPool& queryPool, uint32_t firstQuery, uint32_t queryCount);

	/// <summary>Resets a particular range of queries for a particular QueryPool and sets their status' to unavailable which also makes their numerical results undefined.</summary>
	/// <param name="queryPool">Specifies the query pool managing the queries being reset.</param>
	/// <param name="queryIndex">The query to reset.</param>
	void resetQueryPool(QueryPool& queryPool, uint32_t queryIndex);

	/// <summary>Begins a query for a particular QueryPool.</summary>
	/// <param name="queryPool">Specifies the query pool which will manage the results of the query.</param>
	/// <param name="queryIndex">The query index within the QueryPool which will contain the results.</param>
	/// <param name="flags">Specifies the Query Control Flag bits which provide constraints on the type of queries that can be performed.</param>
	void beginQuery(QueryPool& queryPool, uint32_t queryIndex, QueryControlFlags flags = QueryControlFlags(0));

	/// <summary>Ends a query for a particular QueryPool.</summary>
	/// <param name="queryPool">Specifies the query pool which will manage the results of the query.</param>
	/// <param name="queryIndex">The query index within the QueryPool which will contain the results.</param>
	void endQuery(QueryPool& queryPool, uint32_t queryIndex);

	/// <summary>Copies the query statuses and numerical results directly to buffer memory.</summary>
	/// <param name="queryPool">Specifies the query pool which will manage the results of the query.</param>
	/// <param name="firstQuery">The first query index within the QueryPool which will contain the results.</param>
	/// <param name="queryCount">The number of queries.</param>
	/// <param name="dstBuffer">A buffer object which will receive the results of the copy command.</param>
	/// <param name="offset">An offset into dstBuffer.</param>
	/// <param name="stride">The stride in bytes between results for individual queries within dstBuffer.</param>
	/// <param name="flags">Specifies how and when the results are returned.</param>
	void copyQueryPoolResults(QueryPool& queryPool, uint32_t firstQuery, uint32_t queryCount, Buffer& dstBuffer, VkDeviceSize offset, VkDeviceSize stride, QueryResultFlags flags);

	/// <summary>Requests a timestamp for a particular QueryPool to be written to the query.</summary>
	/// <param name="queryPool">Specifies the query pool which will manage the results of the query.</param>
	/// <param name="queryIndex">The query index within the QueryPool which will contain the results.</param>
	/// <param name="pipelineStage">Specifies the stage of the pipeline to write a timestamp for.</param>
	void writeTimestamp(QueryPool& queryPool, uint32_t queryIndex, PipelineStageFlags pipelineStage);

	/// <summary>Queries if a command buffer is in the recording state</summary>
	/// <returns>True if recording, false otherwise</returns>
	bool isRecording() { return _isRecording; }

	/// <summary>Bind a graphics pipeline.</summary>
	/// <param name="pipeline">The GraphicsPipeline to bind.</param>
	void bindPipeline(const GraphicsPipeline& pipeline)
	{
		_objectReferences.emplace_back(pipeline);
		getDevice()->getVkBindings().vkCmdBindPipeline(getVkHandle(), static_cast<VkPipelineBindPoint>(PipelineBindPoint::e_GRAPHICS), pipeline->getVkHandle());
	}

	/// <summary>Bind a compute pipeline</summary>
	/// <param name="pipeline">The ComputePipeline to bind</param>
	void bindPipeline(ComputePipeline& pipeline)
	{
		_objectReferences.emplace_back(pipeline);
		getDevice()->getVkBindings().vkCmdBindPipeline(getVkHandle(), static_cast<VkPipelineBindPoint>(PipelineBindPoint::e_COMPUTE), pipeline->getVkHandle());
	}

	/// <summary>Bind descriptorsets</summary>
	/// <param name="bindingPoint">Pipeline binding point</param>
	/// <param name="pipelineLayout">Pipeline layout</param>
	/// <param name="firstSet">The set number of the first descriptor set to be bound</param>
	/// <param name="sets">Pointer to the descriptor sets to be bound</param>
	/// <param name="numDescriptorSets">Number of descriptor sets</param>
	/// <param name="dynamicOffsets">Pointer to an array of uint</param>32_t values specifying dynamic offsets
	/// <param name="numDynamicOffsets">Number of dynamic offsets</param>
	void bindDescriptorSets(PipelineBindPoint bindingPoint, const PipelineLayout& pipelineLayout, uint32_t firstSet, const DescriptorSet* sets, uint32_t numDescriptorSets,
		const uint32_t* dynamicOffsets = nullptr, uint32_t numDynamicOffsets = 0);

	/// <summary>Bind descriptorset</summary>
	/// <param name="bindingPoint">Pipeline binding point</param>
	/// <param name="pipelineLayout">Pipeline layout</param>
	/// <param name="firstSet">The set number of the first descriptor set to be bound</param>
	/// <param name="set">Descriptor set to be bound</param>
	/// <param name="dynamicOffsets">Pointer to an array of uint</param>32_t values specifying dynamic offsets
	/// <param name="numDynamicOffsets">Number of dynamic offsets</param>
	void bindDescriptorSet(PipelineBindPoint bindingPoint, const PipelineLayout& pipelineLayout, uint32_t firstSet, const DescriptorSet set,
		const uint32_t* dynamicOffsets = nullptr, uint32_t numDynamicOffsets = 0)
	{
		bindDescriptorSets(bindingPoint, pipelineLayout, firstSet, &set, 1, dynamicOffsets, numDynamicOffsets);
	}

	/// <summary>Bind vertex buffer</summary>
	/// <param name="buffers">A set of vertex buffers to bind</param>
	/// <param name="firstBinding">The first index into buffers</param>
	/// <param name="bindingCount">The number of vertex buffers to bind</param>
	/// <param name="offsets">A pointer to an array of bindingCount buffer offsets</param>
	void bindVertexBuffers(const Buffer* buffers, uint32_t firstBinding, uint16_t bindingCount, const uint32_t* offsets = nullptr)
	{
		VkBuffer native_buffers[static_cast<uint32_t>(FrameworkCaps::MaxVertexBindings)] = { VK_NULL_HANDLE };
		for (uint32_t i = 0; i < bindingCount; ++i)
		{
			_objectReferences.emplace_back(buffers[i]);
			native_buffers[i] = buffers[i]->getVkHandle();
		}

		getDevice()->getVkBindings().vkCmdBindVertexBuffers(getVkHandle(), firstBinding, bindingCount, native_buffers, (VkDeviceSize*)offsets);
	}

	/// <summary>Bind vertex buffer</summary>
	/// <param name="buffer">Buffer</param>
	/// <param name="offset">Buffer offset</param>
	/// <param name="bindingIndex">The index of the vertex input binding whose state is updated by the command.</param>
	void bindVertexBuffer(const Buffer& buffer, uint32_t offset, uint16_t bindingIndex)
	{
		_objectReferences.emplace_back(buffer);
		VkDeviceSize offs = offset;
		getDevice()->getVkBindings().vkCmdBindVertexBuffers(getVkHandle(), bindingIndex, !!buffer, (buffer ? &buffer->getVkHandle() : NULL), &offs);
	}

	/// <summary>Bind vertex buffer</summary>
	/// <param name="buffers">Buffers to be bound</param>
	/// <param name="offsets">Pointer to an array of buffer offsets.</param>
	/// <param name="numBuffers">number of buffers</param>
	/// <param name="startBinding">The indices of the first vertex input binding whose state is updated by the command.</param>
	/// <param name="numBindings">Number of bindings</param>
	void bindVertexBuffer(Buffer const* buffers, uint32_t* offsets, uint16_t numBuffers, uint16_t startBinding, uint16_t numBindings);

	/// <summary>Bind index bufer</summary>
	/// <param name="buffer">Imdex buffer</param>
	/// <param name="offset">Buffer offset</param>
	/// <param name="indexType">IndexType</param>
	void bindIndexBuffer(const Buffer& buffer, uint32_t offset, IndexType indexType)
	{
		_objectReferences.emplace_back(buffer);
		getDevice()->getVkBindings().vkCmdBindIndexBuffer(getVkHandle(), buffer->getVkHandle(), offset, static_cast<VkIndexType>(indexType));
	}

	/// <summary>Add a memory barrier to the command stream, forcing preceeding commands to be written before
	/// succeeding commands are executed.</summary>
	/// <param name="srcStage">A bitmask of PipelineStageFlags specifying the src stage mask.</param>
	/// <param name="dstStage">A bitmask of PipelineStageFlags specifying the dst stage mask.</param>
	/// <param name="barriers">A set of memory barriers to be used in the pipeline barrier.</param>
	/// <param name="dependencyByRegion">A Specifes whether the dependencies in terms of how the execution and memory dependencies are formed.</param>
	void pipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers, bool dependencyByRegion = true);

	/// <summary>Defines a memory dependency between prior event signal operations and subsequent commands.</summary>
	/// <param name="event">The event object to wait on.</param>
	/// <param name="srcStage">A bitmask of PipelineStageFlags specifying the src stage mask.</param>
	/// <param name="dstStage">A bitmask of PipelineStageFlags specifying the dst stage mask.</param>
	/// <param name="barriers">A set of memory barriers to be used in the pipeline barrier.</param>
	void waitForEvent(const Event& event, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);

	/// <summary>Defines a set of memory dependencies between prior event signal operations and subsequent commands.</summary>
	/// <param name="events">A pointer to an array of Event objects to wait on.</param>
	/// <param name="numEvents">The number of event objects to wait on.</param>
	/// <param name="srcStage">A bitmask of PipelineStageFlags specifying the src stage mask.</param>
	/// <param name="dstStage">A bitmask of PipelineStageFlags specifying the dst stage mask.</param>
	/// <param name="barriers">A set of .</param>
	void waitForEvents(const Event* events, uint32_t numEvents, PipelineStageFlags srcStage, PipelineStageFlags dstStage, const MemoryBarrierSet& barriers);

	/// <summary>Defines an execution dependency on commands that were submitted before it, and defines an event signal operation
	/// which sets the event to the signaled state.</summary>
	/// <param name="event">The event object that will be signaled.</param>
	/// <param name="pipelineStageFlags">Specifies the src stage mask used to determine when the event is signaled.</param>
	void setEvent(Event& event, PipelineStageFlags pipelineStageFlags = PipelineStageFlags::e_ALL_COMMANDS_BIT)
	{
		_objectReferences.emplace_back(event);
		getDevice()->getVkBindings().vkCmdSetEvent(getVkHandle(), event->getVkHandle(), static_cast<VkPipelineStageFlags>(pipelineStageFlags));
	}

	/// <summary>Defines an execution dependency on commands that were submitted before it, and defines an event unsignal
	/// operation which resets the event to the unsignaled state.</summary>
	/// <param name="event">The event object that will be unsignaled.</param>
	/// <param name="pipelineStageFlags">Is a bitmask of PipelineStageFlags specifying the src stage mask used to determine when the event is unsignaled.</param>
	void resetEvent(Event& event, PipelineStageFlags pipelineStageFlags = PipelineStageFlags::e_ALL_COMMANDS_BIT)
	{
		getDevice()->getVkBindings().vkCmdResetEvent(getVkHandle(), event->getVkHandle(), static_cast<VkPipelineStageFlags>(pipelineStageFlags));
	}

	/// <summary>Clears this CommandBuffer discarding any previously recorded commands and puts the command buffer in the initial state.
	/// <param name="resetFlags">Is a bitmask of CommandBufferResetFlagBits controlling the reset operation.</param>
	void reset(CommandBufferResetFlags resetFlags = CommandBufferResetFlags::e_NONE)
	{
		_objectReferences.clear();

		getDevice()->getVkBindings().vkResetCommandBuffer(getVkHandle(), static_cast<VkCommandBufferResetFlagBits>(resetFlags));
	}

	/// <summary>Copy data between Images</summary>
	/// <param name="srcImage">Source image</param>
	/// <param name="dstImage">Destination image</param>
	/// <param name="srcImageLayout">Source image layout</param>
	/// <param name="dstImageLayout">Destination image layout</param>
	/// <param name="regions">Regions to copy</param>
	/// <param name="numRegions">Number of regions</param>
	void copyImage(const Image& srcImage, const Image& dstImage, ImageLayout srcImageLayout, ImageLayout dstImageLayout, uint32_t numRegions, const ImageCopy* regions);

	/// <summary>Copy image to buffer</summary>
	/// <param name="srcImage">Source image to copy from</param>
	/// <param name="srcImageLayout">Current src image layout</param>
	/// <param name="dstBuffer">Destination buffer</param>
	/// <param name="regions">Regions to copy</param>
	/// <param name="numRegions">Number of regions</param>
	void copyImageToBuffer(const Image& srcImage, ImageLayout srcImageLayout, Buffer& dstBuffer, const BufferImageCopy* regions, uint32_t numRegions);

	/// <summary>Copy Buffer</summary>
	/// <param name="srcBuffer">Source buffer</param>
	/// <param name="dstBuffer">Destination buffer</param>
	/// <param name="numRegions">Number of regions to copy</param>
	/// <param name="regions">Pointer to an array of BufferCopy structures specifying the regions to copy.
	/// Each region in pRegions is copied from the source buffer to the same region of the destination buffer.
	/// srcBuffer and dstBuffer can be the same buffer or alias the same memory, but the result is undefined if the copy regions overlap in memory.</param>
	void copyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, uint32_t numRegions, const BufferCopy* regions);

	/// <summary>Copy buffer to image</summary>
	/// <param name="buffer">Source Buffer</param>
	/// <param name="image">Destination image</param>
	/// <param name="dstImageLayout">Destination image's current layout</param>
	/// <param name="regionsCount">Copy regions</param>
	/// <param name="regions">Number of regions</param>
	void copyBufferToImage(const Buffer& buffer, const Image& image, ImageLayout dstImageLayout, uint32_t regionsCount, const BufferImageCopy* regions);

	/// <summary>Clear buffer data</summary>
	/// <param name="dstBuffer">Destination buffer to be filled</param>
	/// <param name="dstOffset">The byte offset into the buffer at which to start filling.</param>
	/// <param name="data">A 4-byte word written repeatedly to the buffer to fill size bytes of data.
	/// The data word is written to memory according to the host endianness.</param>
	/// <param name="size">The number of bytes to fill, and must be either a multiple of 4, or VK_WHOLE_SIZE to
	/// fill the range from offset to the end of the buffer</param>
	void fillBuffer(const Buffer& dstBuffer, uint32_t dstOffset, uint32_t data, uint64_t size = VK_WHOLE_SIZE);

	/// <summary>Set viewport</summary>
	/// <param name="viewport">Viewport</param>
	void setViewport(const Viewport& viewport);

	/// <summary>Clear a set of attacments using a number of regions for each selected attachment to clear whilst inside a renderpass.</summary>
	/// <param name="numAttachments">The number of entries in the clearAttachments array.</param>
	/// <param name="clearAttachments">Is a pointer to an array of ClearAttachment structures which defines the attachments
	/// to clear and the clear values to use.</param>
	/// <param name="numRectangles">Is the number of entries in the clearRects array.</param>
	/// <param name="clearRectangles">Points to an array of ClearRect structures defining regions within each selected attachment to clear.</param>
	void clearAttachments(const uint32_t numAttachments, const ClearAttachment* clearAttachments, uint32_t numRectangles, const ClearRect* clearRectangles);

	/// <summary>Clears a particular attachment using a provided region whilst inside of a renderpass.</summary>
	/// <param name="clearAttachment">A single ClearAttachment structure defining the attachment to clear and the clear value to use</param>
	/// <param name="clearRectangle">A ClearRect structure defining a region within the attachment to clear</param>
	void clearAttachment(const ClearAttachment& clearAttachment, const ClearRect& clearRectangle) { clearAttachments(1, &clearAttachment, 1, &clearRectangle); }

	/// <summary>Non-indexed drawing command.</summary>
	/// <param name="firstVertex">The index of the first vertex to draw.</param>
	/// <param name="numVertices">The number of vertices to draw.</param>
	/// <param name="firstInstance">The instance ID of the first instance to draw.</param>
	/// <param name="numInstances">The number of instances to draw.</param>
	void draw(uint32_t firstVertex, uint32_t numVertices, uint32_t firstInstance = 0, uint32_t numInstances = 1);

	/// <summary>Indexed drawing command.</summary>
	/// <param name="firstIndex">The base index within the index buffer.</param>
	/// <param name="numIndices">The number of vertices to draw.</param>
	/// <param name="vertexOffset">The value added to the vertex index before indexining into the vertex buffer.</param>
	/// <param name="firstInstance">The instance ID of the first instance to draw.</param>
	/// <param name="numInstances">The number of instances to draw.</param>
	void drawIndexed(uint32_t firstIndex, uint32_t numIndices, int32_t vertexOffset = 0, uint32_t firstInstance = 0, uint32_t numInstances = 1);

	/// <summary>Non-indexed indirect drawing command.</summary>
	/// <param name="buffer">The buffer containing draw parameters.</param>
	/// <param name="offset">The byte offset into buffer where parameters begin.</param>
	/// <param name="count">The number of draws to execute.</param>
	/// <param name="stride">The byte stride between successive sets of draw commands.</param>
	void drawIndirect(const Buffer& buffer, uint32_t offset, uint32_t count, uint32_t stride);

	/// <summary>Non-indexed indirect drawing command.</summary>
	/// <param name="buffer">The buffer containing draw parameters.</param>
	/// <param name="offset">The byte offset into buffer where parameters begin.</param>
	/// <param name="count">The number of draws to execute.</param>
	/// <param name="stride">The byte stride between successive sets of draw commands.</param>
	void drawIndexedIndirect(const Buffer& buffer, uint32_t offset, uint32_t count, uint32_t stride);

	/// <summary>Dispatching work provokes work in a compute pipeline. A compute pipeline must be bound to the command buffer
	/// before any dispatch commands are recorded.</summary>
	/// <param name="numGroupX">The number of local workgroups to dispatch in the X dimension.</param>
	/// <param name="numGroupY">The number of local workgroups to dispatch in the Y dimension.</param>
	/// <param name="numGroupZ">The number of local workgroups to dispatch in the Z dimension.</param>
	void dispatch(uint32_t numGroupX, uint32_t numGroupY, uint32_t numGroupZ);

	/// <summary>Dispatching work provokes work in a compute pipeline. A compute pipeline must be bound to the command buffer
	/// before any dispatch commands are recorded. dispatchIndirect behaves similarly to dispatch except that the parameters
	/// are read by the device from a buffer during execution. The parameters of the dispatch are encoded in a DispatchIndirectCommand
	/// structure taken from buffer starting at offset</summary>
	/// <param name="buffer">The buffer containing dispatch parameters.</param>
	/// <param name="offset">The byte offset into buffer where parameters begin.</param>
	void dispatchIndirect(Buffer& buffer, uint32_t offset);

	/// <summary>Clears a color image outside of a renderpass instance.</summary>
	/// <param name="image">Image to clear</param>
	/// <param name="clearColor">Clear color value</param>
	/// <param name="currentLayout">Image current layout</param>
	/// <param name="baseMipLevel">Base mip map level to clear</param>
	/// <param name="numLevels">Number of mipmap levels to clear</param>
	/// <param name="baseArrayLayer">Base array layer to clear</param>
	/// <param name="numLayers">Number of array layers to clear</param>
	void clearColorImage(const ImageView& image, const ClearColorValue& clearColor, ImageLayout currentLayout, const uint32_t baseMipLevel = 0, const uint32_t numLevels = 1,
		const uint32_t baseArrayLayer = 0, const uint32_t numLayers = 1);

	/// <summary>Clears a color image outside of a renderpass instance using a number of ranges.</summary>
	/// <param name="image">Image to clear.</param>
	/// <param name="clearColor">Clear color value.</param>
	/// <param name="currentLayout">Image current layout.</param>
	/// <param name="baseMipLevels">Base mip map level to clear.</param>
	/// <param name="numLevels">A pointer to an array of a number of mipmap levels to clear.</param>
	/// <param name="baseArrayLayers">A pointer to an array of base array layers to clear.</param>
	/// <param name="numLayers">A pointer to an array array layers to clear.</param>
	/// <param name="numRanges">The number of elements in the baseMipLevel, numLevels, baseArrayLayers and numLayers arrays.</param>
	void clearColorImage(const ImageView& image, const ClearColorValue& clearColor, ImageLayout currentLayout, const uint32_t* baseMipLevels, const uint32_t* numLevels,
		const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges);

	/// <summary>Clear depth stencil image outside of a renderpass instance.</summary>
	/// <param name="image">Image to clear</param>
	/// <param name="clearDepth">Clear depth value</param>
	/// <param name="clearStencil">Clear stencil value</param>
	/// <param name="baseMipLevel">Base mip map level to clear</param>
	/// <param name="numLevels">Number of mipmap levels to clear</param>
	/// <param name="baseArrayLayer">Base array layer to clear</param>
	/// <param name="numLayers">Number of array layers to clear</param>
	/// <param name="layout">Image current layout</param>
	void clearDepthStencilImage(const Image& image, float clearDepth, uint32_t clearStencil, const uint32_t baseMipLevel, const uint32_t numLevels, const uint32_t baseArrayLayer,
		const uint32_t numLayers, ImageLayout layout);

	/// <summary>Clear depth stencil image outside of a renderpass instance using a number of ranges.</summary>
	/// <param name="image">Image to clear</param>
	/// <param name="clearDepth">Clear depth value</param>
	/// <param name="clearStencil">Clear stencil value</param>
	/// <param name="baseMipLevels">A pointer to an array of base mip map levels to clear</param>
	/// <param name="numLevels">A pointer to an array of the number of mipmap levels to clear</param>
	/// <param name="baseArrayLayers">A pointer to an array of base array layers to clear</param>
	/// <param name="numLayers">A pointer to an array of the number of layers to clear</param>
	/// <param name="numRanges">A number of ranges of the depth stencil image to clear. This number will be
	/// used as the number of array elements in the arrays passed to baseMipLevels, numLevels,
	/// baseArrayLayers and numLayers</param>
	/// <param name="layout">Image current layout</param>
	void clearDepthStencilImage(const Image& image, float clearDepth, uint32_t clearStencil, const uint32_t* baseMipLevels, const uint32_t* numLevels,
		const uint32_t* baseArrayLayers, const uint32_t* numLayers, uint32_t numRanges, ImageLayout layout);

	/// <summary>Clears a stencil image outside of a renderpass instance.</summary>
	/// <param name="image">Image to clear</param>
	/// <param name="clearStencil">Clear stencil value</param>
	/// <param name="baseMipLevel">Base mip map level to clear</param>
	/// <param name="numLevels">Number of mipmap levels to clear</param>
	/// <param name="baseArrayLayer">Base array layer to clear</param>
	/// <param name="numLayers">Number of array layers to clear</param>
	/// <param name="layout">Image current layout</param>
	void clearStencilImage(const Image& image, uint32_t clearStencil, const uint32_t baseMipLevel, const uint32_t numLevels, const uint32_t baseArrayLayer,
		const uint32_t numLayers, ImageLayout layout);

	/// <summary>Clear stencil image outside of a renderpass instance using a number of ranges.</summary>
	/// <param name="image">Image to clear</param>
	/// <param name="clearStencil">Clear stencil value</param>
	/// <param name="baseMipLevels">A pointer to an array of base mip map levels to clear</param>
	/// <param name="numLevels">A pointer to an array of the number of mipmap levels to clear</param>
	/// <param name="baseArrayLayers">A pointer to an array of base array layers to clear</param>
	/// <param name="numLayers">A pointer to an array of the number of layers to clear</param>
	/// <param name="numRanges">A number of ranges of the stencil image to clear. This number will be
	/// used as the number of array elements in the arrays passed to baseMipLevels, numLevels,
	/// baseArrayLayers and numLayers</param>
	/// <param name="layout">Image current layout</param>
	void clearStencilImage(const Image& image, uint32_t clearStencil, const uint32_t* baseMipLevels, const uint32_t* numLevels, const uint32_t* baseArrayLayers,
		const uint32_t* numLayers, uint32_t numRanges, ImageLayout layout);

	/// <summary>Clear depth image outside of a renderpass instance.</summary>
	/// <param name="image">Image to clear</param>
	/// <param name="clearDepth">Clear value</param>
	/// <param name="baseMipLevel">Base mip map level to clear</param>
	/// <param name="numLevels">Number of mipmap levels to clear</param>
	/// <param name="baseArrayLayer">Base arraylayer to clear</param>
	/// <param name="numLayers">Number of array layers to clear</param>
	/// <param name="layout">Current layout of the image</param>
	void clearDepthImage(
		const Image& image, float clearDepth, const uint32_t baseMipLevel, const uint32_t numLevels, const uint32_t baseArrayLayer, const uint32_t numLayers, ImageLayout layout);

	/// <summary>Clears the depth image outside of a renderpass instance using a number of ranges.</summary>
	/// <param name="image">Image to clear</param>
	/// <param name="clearDepth">Clear depth value</param>
	/// <param name="baseMipLevels">A pointer to an array of base mip map levels to clear</param>
	/// <param name="numLevels">A pointer to an array of the number of mipmap levels to clear</param>
	/// <param name="baseArrayLayers">A pointer to an array of base array layers to clear</param>
	/// <param name="numLayers">A pointer to an array of the number of layers to clear</param>
	/// <param name="numRanges">A number of ranges of the stencil image to clear. This number will be
	/// used as the number of array elements in the arrays passed to baseMipLevels, numLevels,
	/// baseArrayLayers and numLayers</param>
	/// <param name="layout">Image current layout</param>
	void clearDepthImage(const Image& image, float clearDepth, const uint32_t* baseMipLevels, const uint32_t* numLevels, const uint32_t* baseArrayLayers, const uint32_t* numLayers,
		uint32_t numRanges, ImageLayout layout);

	/// <summary>Sets the dynamic scissor state affecting pipeline objects created with VK_DYNAMIC_STATE_SCISSOR enabled.</summary>
	/// <param name="firstScissor">The index of the first scissor whose state is updated.</param>
	/// <param name="numScissors">The number of scissors whose rectangles are updated.</param>
	/// <param name="scissors">A pointer to an array of Rect2Di structures defining scissor rectangles.</param>
	void setScissor(uint32_t firstScissor, uint32_t numScissors, const Rect2D* scissors);

	/// <summary>Sets the dynamic depth bounds state affecting pipeline objects created with VK_DYNAMIC_STATE_DEPTH_BOUNDS enabled.</summary>
	/// <param name="min">The lower bound of the range of depth values used in the depth bounds test.</param>
	/// <param name="max">The upper bound of the range.</param>
	void setDepthBounds(float min, float max);

	/// <summary>Sets the dynamic stencil write mask state affecting pipeline objects created with VK_DYNAMIC_STATE_STENCIL_WRITE_MASK enabled.</summary>
	/// <param name="face">A bitmask of StencilFaceFlags specifying the set of stencil state for which to update the write mask.</param>
	/// <param name="writeMask">The new value to use as the stencil write mask</param>
	void setStencilWriteMask(StencilFaceFlags face, uint32_t writeMask);

	/// <summary>Sets the dynamic stencil reference mask state affecting pipeline objects created with VK_DYNAMIC_STATE_STENCIL_REFERENCE enabled.</summary>
	/// <param name="face">A bitmask of StencilFaceFlags specifying the set of stencil state for which to update the reference value.</param>
	/// <param name="reference">The new value to use as the stencil reference value.</param>
	void setStencilReference(StencilFaceFlags face, uint32_t reference);

	/// <summary>Sets the dynamic stencil compare mask state affecting pipeline objects created with VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK enabled.</summary>
	/// <param name="face">A bitmask of StencilFaceFlags specifying the set of stencil state for which to update the compare mask.</param>
	/// <param name="compareMask">The new value to use as the stencil compare value.</param>
	void setStencilCompareMask(StencilFaceFlags face, uint32_t compareMask);

	/// <summary>Sets the dynamic depth bias state affecting pipeline objects created where depthBiasEnable is enabled.</summary>
	/// <param name="constantFactor">A scalar factor controlling the constant depth value added to each fragment.</param>
	/// <param name="clamp">The maximum (or minimum) depth bias of a fragment.</param>
	/// <param name="slopeFactor">A scalar factor applied to a fragment's slope in depth bias calculations.</param>
	void setDepthBias(float constantFactor, float clamp, float slopeFactor);

	/// <summary>Sets the dynamic blend constant bias state affecting pipeline objects created with VK_DYNAMIC_STATE_BLEND_CONSTANTS enabled.</summary>
	/// <param name="rgba">An array of four values specifying the R, G, B, and A components of the blend constant color used in blending,
	/// depending on the blend factor</param>
	void setBlendConstants(float rgba[4]);

	/// <summary>Sets the dynamic line width state affecting pipeline objects created with VK_DYNAMIC_STATE_LINE_WIDTH enabled.</summary>
	/// <param name="lineWidth">The width of rasterized line segments.</param>
	void setLineWidth(float lineWidth);

	/// <summary>Copies regions of a src image into a dst image, potentially also performing format conversions, aritrary scaling and filtering.</summary>
	/// <param name="srcImage">The src Image in the copy.</param>
	/// <param name="dstImage">The dst image.</param>
	/// <param name="regions">A pointer to an array of ImageBlitRange structures specifying the regions to blit.</param>
	/// <param name="numRegions">The number of regions to blit.</param>
	/// <param name="filter">A Filter specifying the filter to apply if the blits require scaling</param>
	/// <param name="srcLayout">The layout of the src image subresrcs for the blit.</param>
	/// <param name="dstLayout">The layout of the dst image subresrcs for the blit.</param>
	void blitImage(const Image& srcImage, const Image& dstImage, const ImageBlit* regions, uint32_t numRegions, Filter filter, ImageLayout srcLayout, ImageLayout dstLayout);

	/// <summary>Copies regions of a src image into a dst image, potentially also performing format conversions, aritrary scaling and filtering.</summary>
	/// <param name="srcImage">The src Image in the copy.</param>
	/// <param name="dstImage">The dst image.</param>
	/// <param name="regions">A pointer to an array of ImageBlitRange structures specifying the regions to blit.</param>
	/// <param name="numRegions">The number of regions to blit.</param>
	/// <param name="srcLayout">The layout of the src image subresrcs for the blit.</param>
	/// <param name="dstLayout">The layout of the dst image subresrcs for the blit.</param>
	void resolveImage(const Image& srcImage, const Image& dstImage, const ImageResolve* regions, uint32_t numRegions, ImageLayout srcLayout, ImageLayout dstLayout);

	/// <summary>Updates buffer data inline in a command buffer. The update is only allowed outside of a renderpass and is treated as a transfer operation
	/// for the purposes of syncrhonization.</summary>
	/// <param name="buffer">The buffer to be updated.</param>
	/// <param name="data">A pointer to the src data for the buffer update. The data must be at least length bytes in size.</param>
	/// <param name="offset">The byte offset into the buffer to start updating, and must be a multiple of 4.</param>
	/// <param name="length">The number of bytes to update, and must be a multiple of 4.</param>
	void updateBuffer(const Buffer& buffer, const void* data, uint32_t offset, uint32_t length);

	/// <summary>Updates the value of shader push constants at the offset specified.</summary>
	/// <param name="pipelineLayout">The pipeline layout used to program the push constant updates.</param>
	/// <param name="stageFlags">A bitmask of ShaderStageFlag specifying the shader stages that will use the push constants in the updated range.</param>
	/// <param name="offset">The start offset of the push constant range to update, in units of bytes.</param>
	/// <param name="size">The size of the push constant range to update, in units of bytes.</param>
	/// <param name="data">An array of size bytes containing the new push constant values.</param>
	void pushConstants(const PipelineLayout& pipelineLayout, ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);

	/// <summary>Binds a transform feedback buffer to the command buffer for use in subsequent draw commands.</summary>
	/// <param name="buffer">A buffer to bind the command buffer.</param>
	/// <param name="offset">A buffer offset.</param>
	/// <param name="size">An optional buffer size, which specifies the maximum number of bytes to capture to the corresponding transform feedback buffer.</param>
	void bindTransformFeedbackBuffers(pvrvk::Buffer buffer, VkDeviceSize offset, VkDeviceSize size = VK_WHOLE_SIZE);

	/// <summary>Binds a set of transform feedback buffers to the command buffer for use in subsequent draw commands.</summary>
	/// <param name="firstBinding">The index of the first transform feedback binding whose state is updated by the command.</param>
	/// <param name="bindingCount">The number of transform feedback bindings whose state is updated by the command.</param>
	/// <param name="buffers">A list of buffers to bind the command buffer.</param>
	/// <param name="offsets">Pointer to an array of buffer offsets.</param>
	/// <param name="sizes">An optional array of buffer sizes, which specifies the maximum number of bytes to capture to the corresponding transform feedback buffer.</param>
	void bindTransformFeedbackBuffers(uint32_t firstBinding, uint32_t bindingCount, const pvrvk::Buffer* buffers, const VkDeviceSize* offsets, const VkDeviceSize* sizes = nullptr);

	/// <summary>Makes active transform feedback for specific transform feedback buffers.</summary>
	/// <param name="firstCounterBuffer">Index of the first transform feedback buffer.</param>
	/// <param name="numCounterBuffers">The number of buffers.</param>
	/// <param name="counterBuffers">An optional list of buffers where the handles of the buffers correspond to the counter buffers which contain a 4 byte
	/// integer value representing the byte offset from the start of the corresponding transform feedback buffer from where to start capturing vertex data.</param>
	/// <param name="counterBufferOffsets">An optional array of offsets within each of the pCounterBuffers where the counter values were previously written.</param>
	void beginTransformFeedback(
		uint32_t firstCounterBuffer, uint32_t numCounterBuffers, const pvrvk::Buffer* counterBuffers = nullptr, const VkDeviceSize* counterBufferOffsets = nullptr);

	/// <summary>Makes active transform feedback for specific transform feedback buffers.</summary>
	/// <param name="counterBuffer">The handle of the buffer correspond to the counter buffer which contains a 4 byte
	/// integer value representing the byte offset from the start of the corresponding transform feedback buffer from where to start capturing vertex data.</param>
	/// <param name="counterBufferOffset">An optional offset within the counterBuffer where the counter values were previously written.</param>
	void beginTransformFeedback(pvrvk::Buffer counterBuffer, VkDeviceSize counterBufferOffset = 0);

	/// <summary>Makes inactive transform feedback for specific transform feedback buffers.</summary>
	/// <param name="firstCounterBuffer">Index of the first transform feedback buffer.</param>
	/// <param name="numCounterBuffers">The number of buffers.</param>
	/// <param name="counterBuffers">An optional list of buffers where the handles of the buffers correspond to the counter buffers which contain a 4 byte
	/// integer value representing the byte offset from the start of the corresponding transform feedback buffer from where to start capturing vertex data.</param>
	/// <param name="counterBufferOffsets">An optional array of offsets within each of the pCounterBuffers where the counter values were previously written.</param>
	void endTransformFeedback(uint32_t firstCounterBuffer, uint32_t numCounterBuffers, const pvrvk::Buffer* counterBuffers = nullptr, const VkDeviceSize* counterBufferOffsets = nullptr);

	/// <summary>Makes inactive transform feedback for specific transform feedback buffers.</summary>
	/// <param name="counterBuffer">The handle of the buffer correspond to the counter buffer which contains a 4 byte
	/// integer value representing the byte offset from the start of the corresponding transform feedback buffer from where to start capturing vertex data.</param>
	/// <param name="counterBufferOffset">An optional offset within the counterBuffer where the counter values were previously written.</param>
	void endTransformFeedback(pvrvk::Buffer counterBuffer, VkDeviceSize counterBufferOffset = 0);

	/// <summary>Begins a query for a particular QueryPool.</summary>
	/// <param name="queryPool">Specifies the query pool which will manage the results of the query.</param>
	/// <param name="queryIndex">The query index within the QueryPool which will contain the results.</param>
	/// <param name="flags">Specifies the Query Control Flag bits which provide constraints on the type of queries that can be performed.</param>
	/// <param name="index">The query type specific index.</param>
	void beginQueryIndexed(QueryPool& queryPool, uint32_t queryIndex, QueryControlFlags flags = QueryControlFlags(0), uint32_t index = 0);

	/// <summary>Ends a query for a particular QueryPool.</summary>
	/// <param name="queryPool">Specifies the query pool which will manage the results of the query.</param>
	/// <param name="queryIndex">The query index within the QueryPool which will contain the results.</param>
	/// <param name="index">The query type specific index.</param>
	void endQueryIndexed(QueryPool& queryPool, uint32_t queryIndex, uint32_t index = 0);

	/// <summary>Records a non-indexed draw call, where the vertex count is based on a byte count read from a buffer and the passed in vertex stride parameter.</summary>
	/// <param name="instanceCount">The number of instances to draw.</param>
	/// <param name="firstInstance">The instance ID of the first instance to draw.</param>
	/// <param name="counterBuffer">The buffer handle from where the byte count is read.</param>
	/// <param name="counterBufferOffset">The offset into the buffer used to read the byte count, which is used to calculate the vertex count for this draw call.</param>
	/// <param name="counterOffset">Is subtracted from the byte count read from the counterBuffer at the counterBufferOffset.</param>
	/// <param name="vertexStride">The stride in bytes between each element of the vertex data that is used to calculate the vertex count from the counter value</param>
	void drawIndirectByteCount(
		uint32_t instanceCount, uint32_t firstInstance, pvrvk::Buffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride);

	/// <summary>Const getter for the command pool used to allocate this command buffer.</summary>
	/// <returns>The command pool used to allocate this command buffer.</returns>
	const CommandPool getCommandPool() const { return _pool; }
};

/// <summary>Contains all the commands and states that need to be recorded for later submission to the gpu including pipelines,
/// textures, descriptor sets. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer.</summary>
class CommandBuffer_ : public CommandBufferBase_
{
protected:
	friend class CommandPool_;

	/// <summary>A class which restricts the creation of a pvrvk::CommandBuffer to children or friends of a pvrvk::impl::CommandBuffer_.</summary>
	class make_shared_enabler : public CommandBufferBase_::make_shared_enabler
	{
	protected:
		/// <summary>Constructor for a make_shared_enabler.</summary>
		make_shared_enabler() : CommandBufferBase_::make_shared_enabler() {}
		/// <summary>Indicates that a pvrvk::impl::CommandBuffer_ is a friend.</summary>
		friend CommandBuffer_;
	};

	/// <summary>Protected function used to create a pvrvk::CommandBuffer. Note that this function shouldn't normally be called
	/// directly and will be called by a friend of CommandBuffer_ which will generally be a CommandPool</summary>
	/// <param name="device">The device used to allocate the secondary command buffer.</param>
	/// <param name="pool">The pool from which the command buffer will be allocated.</param>
	/// <param name="myHandle">The vulkan handle for this command buffer.</param>
	/// <returns>Returns a successfully created pvrvk::CommandBuffer</returns>
	static CommandBuffer constructShared(const DeviceWeakPtr& device, CommandPool pool, VkCommandBuffer myHandle)
	{
		return std::make_shared<CommandBuffer_>(make_shared_enabler{}, device, pool, myHandle);
	}

#ifdef DEBUG
	//!\cond NO_DOXYGEN
	pvrvk::Framebuffer _currentlyBoundFramebuffer;
	uint32_t _currentSubpass;
	//!\endcond
#endif

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(CommandBuffer_)

	CommandBuffer_(make_shared_enabler, const DeviceWeakPtr& device, CommandPool pool, VkCommandBuffer myHandle)
		: CommandBufferBase_(make_shared_enabler{}, device, pool, myHandle)
#ifdef DEBUG
		  ,
		  _currentSubpass(static_cast<uint32_t>(-1))
#endif
	{}

	/// <summary>Destructor. Virtual (for polymorphic use).</summary>
	virtual ~CommandBuffer_()
	{
#ifdef DEBUG
		_currentlyBoundFramebuffer.reset();
#endif
	}
	//!\endcond

	/// <summary>Record commands from the secondary command buffer.</summary>
	/// <param name="secondaryCmdBuffer">Record all commands from a secondary command buffer</param>
	void executeCommands(const SecondaryCommandBuffer& secondaryCmdBuffer);

	/// <summary>Record commands from an array of secondary command buffer</summary>
	/// <param name="secondaryCmdBuffers">A c-style array of SecondaryCommandBuffers</param>
	/// <param name="numCommandBuffers">The number of SecondaryCommandBuffers in secondaryCmdBuffers</param>
	void executeCommands(const SecondaryCommandBuffer* secondaryCmdBuffers, uint32_t numCommandBuffers);

	/// <summary>Begins the renderpass for the provided Framebuffer and renderpass and using a specific renderable area.</summary>
	/// <param name="framebuffer">A Framework wrapped Vulkan Framebuffer object to use as part of the VkRenderPassBeginInfo structure.</param>
	/// <param name="renderPass">A Framework wrapped Vulkan RenderPass object to use as part of the VkRenderPassBeginInfo structure.</param>
	/// <param name="renderArea">Specifies the render area that is affected by the renderpass instance.</param>
	/// <param name="inlineFirstSubpass">Specifies whether the renderpass uses an inline subpass as its first subpass.</param>
	/// <param name="clearValues">A pointer to a list of ClearValue structures which will be used as part of the VkRenderPassBeginInfo structure.</param>
	/// <param name="numClearValues">The number or ClearValue structures passed to the VkRenderPassBeginInfo structure.</param>
	void beginRenderPass(const Framebuffer& framebuffer, const RenderPass& renderPass, const Rect2D& renderArea, bool inlineFirstSubpass = false,
		const ClearValue* clearValues = nullptr, uint32_t numClearValues = 0);

	/// <summary>Begins a renderpass for the provided Framebuffer taking the renderpass from the provided Framebuffer and using a specific renderable area.</summary>
	/// <param name="framebuffer">A Framework wrapped Vulkan Framebuffer object to use as part of the VkRenderPassBeginInfo structure.</param>
	/// <param name="renderArea">Specifies the render area that is affected by the renderpass instance.</param>
	/// <param name="inlineFirstSubpass">Specifies whether the renderpass uses an inline subpass as its first subpass.</param>
	/// <param name="clearValues">A pointer to a list of ClearValue structures which will be used as part of the VkRenderPassBeginInfo structure.</param>
	/// <param name="numClearValues">The number or ClearValue structures passed to the VkRenderPassBeginInfo structure.</param>
	void beginRenderPass(const Framebuffer& framebuffer, const Rect2D& renderArea, bool inlineFirstSubpass = false, const ClearValue* clearValues = nullptr, uint32_t numClearValues = 0);

	/// <summary>Begins a renderpass for the provided Framebuffer taking the renderpass from the provided Framebuffer and taking the renderable area from the Framebuffer.</summary>
	/// <param name="framebuffer">A Framework wrapped Vulkan Framebuffer object to use as part of the VkRenderPassBeginInfo structure.</param>
	/// <param name="inlineFirstSubpass">Specifies whether the renderpass uses an inline subpass as its first subpass.</param>
	/// <param name="clearValues">A pointer to a list of ClearValue structures which will be used as part of the VkRenderPassBeginInfo structure.</param>
	/// <param name="numClearValues">The number or ClearValue structures passed to the VkRenderPassBeginInfo structure.</param>
	void beginRenderPass(const Framebuffer& framebuffer, bool inlineFirstSubpass = false, const ClearValue* clearValues = nullptr, uint32_t numClearValues = 0);

	/// <summary>Finish the a renderpass (executes the StoreOp).</summary>
	void endRenderPass()
	{
		getDevice()->getVkBindings().vkCmdEndRenderPass(getVkHandle());

#ifdef DEBUG
		auto& currentRenderPass = _currentlyBoundFramebuffer->getCreateInfo().getRenderPass();
		assert(currentRenderPass->getCreateInfo().getNumAttachmentDescription() == _currentlyBoundFramebuffer->getNumAttachments());

		for (uint32_t i = 0; i < _currentlyBoundFramebuffer->getNumAttachments(); ++i)
		{ _currentlyBoundFramebuffer->getAttachment(i)->getImage()->setImageLayout(currentRenderPass->getCreateInfo().getAttachmentDescription(i).getFinalLayout()); }

		_currentlyBoundFramebuffer.reset();
		_currentSubpass = static_cast<uint32_t>(-1);
#endif
	}

#ifdef DEBUG
	void updatePerSubpassImageLayouts()
	{
		auto& currentRenderPass = _currentlyBoundFramebuffer->getCreateInfo().getRenderPass();

		for (uint8_t i = 0; i < currentRenderPass->getCreateInfo().getSubpass(_currentSubpass).getNumInputAttachmentReference(); ++i)
		{
			const AttachmentReference& attachmentReference = currentRenderPass->getCreateInfo().getSubpass(_currentSubpass).getInputAttachmentReference(i);
			_currentlyBoundFramebuffer->getCreateInfo().getAttachment(attachmentReference.getAttachment())->getImage()->setImageLayout(attachmentReference.getLayout());
		}

		for (uint8_t i = 0; i < currentRenderPass->getCreateInfo().getSubpass(_currentSubpass).getNumColorAttachmentReference(); ++i)
		{
			const AttachmentReference& attachmentReference = currentRenderPass->getCreateInfo().getSubpass(_currentSubpass).getColorAttachmentReference(i);
			_currentlyBoundFramebuffer->getCreateInfo().getAttachment(attachmentReference.getAttachment())->getImage()->setImageLayout(attachmentReference.getLayout());
		}

		for (uint8_t i = 0; i < currentRenderPass->getCreateInfo().getSubpass(_currentSubpass).getNumResolveAttachmentReference(); ++i)
		{
			const AttachmentReference& attachmentReference = currentRenderPass->getCreateInfo().getSubpass(_currentSubpass).getResolveAttachmentReference(i);
			_currentlyBoundFramebuffer->getCreateInfo().getAttachment(attachmentReference.getAttachment())->getImage()->setImageLayout(attachmentReference.getLayout());
		}
	}
#endif

	/// <summary>Record next sub pass commands from a secondary-commandbuffer.</summary>
	/// <param name="contents">Specifies how the commands in the next subpass will be provided, in the same
	/// fashion as the corresponding parameter of beginRenderPass.</param>
	void nextSubpass(SubpassContents contents)
	{
		getDevice()->getVkBindings().vkCmdNextSubpass(getVkHandle(), static_cast<VkSubpassContents>(contents));

#ifdef DEBUG
		_currentSubpass++;
		updatePerSubpassImageLayouts();
#endif
	}
};

/// <summary>Contains all the commands and states that need to be submitted to the gpu, including pipeline, texture,
/// and samplers. Virtually everything that needs to happen on the GPU is submitted to the CommandBuffer.</summary>
/// <remarks>Secondary command buffers cannot contain RenderPasses, and cannot be submitted to the GPU.
/// SecondaryCommandBuffers can be submitted to the primaryCommandBuffer -It is invalid to submit commands to a
/// command buffer while it is not being recorded. -It is invalid to reset a command buffer while it is being
/// recorded. -It is invalid to submit a command buffer more than once if it is one time submit command buffer
/// -Draw commands must be between a BeginRenderPass and an EndRenderPass command</remarks>
class SecondaryCommandBuffer_ : public CommandBufferBase_
{
protected:
	friend class CommandPool_;

	/// <summary>A class which restricts the creation of a pvrvk::SecondaryCommandBuffer to children or friends of a pvrvk::impl::SecondaryCommandBuffer_.</summary>
	class make_shared_enabler : public CommandBufferBase_::make_shared_enabler
	{
	protected:
		/// <summary>Constructor for a make_shared_enabler.</summary>
		make_shared_enabler() : CommandBufferBase_::make_shared_enabler() {}
		/// <summary>Indicates that a pvrvk::impl::SecondaryCommandBuffer_ is a friend.</summary>
		friend SecondaryCommandBuffer_;
	};

	/// <summary>Protected function used to create a pvrvk::SecondaryCommandBuffer. Note that this function shouldn't normally be called
	/// directly and will be called by a friend of SecondaryCommandBuffer_ which will generally be a CommandPool</summary>
	/// <param name="device">The device used to allocate the secondary command buffer.</param>
	/// <param name="pool">The pool from which the command buffer will be allocated.</param>
	/// <param name="myHandle">The vulkan handle for this command buffer.</param>
	/// <returns>Returns a successfully created pvrvk::SecondaryCommandBuffer</returns>
	static SecondaryCommandBuffer constructShared(const DeviceWeakPtr& device, CommandPool pool, VkCommandBuffer myHandle)
	{
		return std::make_shared<SecondaryCommandBuffer_>(make_shared_enabler{}, device, pool, myHandle);
	}

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(SecondaryCommandBuffer_)

	/// <summary>Constructor for a secondary command buffer. This function should not be called directly and instead constructShared should be called.</summary>
	/// <param name="device">The device used to allocate this secondary command buffer.</param>
	/// <param name="pool">The pool from which the secondary command buffer will be allocated.</param>
	/// <param name="myHandle">The vulkan handle for this secondary command buffer.</param>
	SecondaryCommandBuffer_(make_shared_enabler, const DeviceWeakPtr& device, CommandPool pool, VkCommandBuffer myHandle)
		: CommandBufferBase_(make_shared_enabler{}, device, pool, myHandle)
	{}
	//!\endcond

	using CommandBufferBase_::begin;

	/// <summary>Call this function before beginning to record commands. If the Framebuffer object is known, prefer the Framebuffer overload
	/// as it may offer better performance.</summary>
	/// <param name="renderpass">A RenderPass object defining which render passes this SecondaryCommandBuffer will be compatible with and can be executed within.</param>
	/// <param name="subpass">The index of the subpass within the render pass instance that this CommandBuffer will be executed within.</param>
	/// <param name="flags">Flags is a bitmask of CommandBufferUsageFlagBits specifying usage behavior for the command buffer.</param>
	void begin(const RenderPass& renderpass, uint32_t subpass = 0, const CommandBufferUsageFlags flags = CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);

	/// <summary>Call this function before beginning to record commands.</summary>
	/// <param name="framebuffer">Refers to an Framebuffer object that this CommandBuffer will be rendering to if it is executed within a render pass instance.</param>
	/// <param name="subpass">The index of the subpass within the render pass instance that this CommandBuffer will be executed within.</param>
	/// <param name="flags">Flags is a bitmask of CommandBufferUsageFlagBits specifying usage behavior for the command buffer.</param>
	void begin(const Framebuffer& framebuffer, uint32_t subpass = 0, const CommandBufferUsageFlags flags = CommandBufferUsageFlags::e_RENDER_PASS_CONTINUE_BIT);
};
} // namespace impl
} // namespace pvrvk
