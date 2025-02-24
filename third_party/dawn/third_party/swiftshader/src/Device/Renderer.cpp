// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Renderer.hpp"

#include "Clipper.hpp"
#include "Polygon.hpp"
#include "Primitive.hpp"
#include "Vertex.hpp"
#include "Pipeline/Constants.hpp"
#include "Pipeline/SpirvShader.hpp"
#include "Reactor/Reactor.hpp"
#include "System/Debug.hpp"
#include "System/Half.hpp"
#include "System/Math.hpp"
#include "System/Memory.hpp"
#include "System/Timer.hpp"
#include "Vulkan/VkConfig.hpp"
#include "Vulkan/VkDescriptorSet.hpp"
#include "Vulkan/VkDevice.hpp"
#include "Vulkan/VkFence.hpp"
#include "Vulkan/VkImageView.hpp"
#include "Vulkan/VkPipelineLayout.hpp"
#include "Vulkan/VkQueryPool.hpp"

#include "marl/containers.h"
#include "marl/defer.h"
#include "marl/trace.h"

#undef max

#ifndef NDEBUG
unsigned int minPrimitives = 1;
unsigned int maxPrimitives = 1 << 21;
#endif

namespace sw {

template<typename T>
inline bool setBatchIndices(unsigned int batch[128][3], VkPrimitiveTopology topology, VkProvokingVertexModeEXT provokingVertexMode, T indices, unsigned int start, unsigned int triangleCount)
{
	bool provokeFirst = (provokingVertexMode == VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT);

	switch(topology)
	{
	case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		{
			auto index = start;
			auto pointBatch = &(batch[0][0]);
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				*pointBatch++ = indices[index++];
			}

			// Repeat the last index to allow for SIMD width overrun.
			index--;
			for(unsigned int i = 0; i < 3; i++)
			{
				*pointBatch++ = indices[index];
			}
		}
		break;
	case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		{
			auto index = 2 * start;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index + (provokeFirst ? 0 : 1)];
				batch[i][1] = indices[index + (provokeFirst ? 1 : 0)];
				batch[i][2] = indices[index + 1];

				index += 2;
			}
		}
		break;
	case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		{
			auto index = start;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index + (provokeFirst ? 0 : 1)];
				batch[i][1] = indices[index + (provokeFirst ? 1 : 0)];
				batch[i][2] = indices[index + 1];

				index += 1;
			}
		}
		break;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		{
			auto index = 3 * start;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index + (provokeFirst ? 0 : 2)];
				batch[i][1] = indices[index + (provokeFirst ? 1 : 0)];
				batch[i][2] = indices[index + (provokeFirst ? 2 : 1)];

				index += 3;
			}
		}
		break;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		{
			auto index = start;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index + (provokeFirst ? 0 : 2)];
				batch[i][1] = indices[index + ((start + i) & 1) + (provokeFirst ? 1 : 0)];
				batch[i][2] = indices[index + (~(start + i) & 1) + (provokeFirst ? 1 : 0)];

				index += 1;
			}
		}
		break;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		{
			auto index = start + 1;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][provokeFirst ? 0 : 2] = indices[index + 0];
				batch[i][provokeFirst ? 1 : 0] = indices[index + 1];
				batch[i][provokeFirst ? 2 : 1] = indices[0];

				index += 1;
			}
		}
		break;
	default:
		ASSERT(false);
		return false;
	}

	return true;
}

DrawCall::DrawCall()
{
	// TODO(b/140991626): Use allocateUninitialized() instead of allocateZeroOrPoison() to improve startup peformance.
	data = (DrawData *)sw::allocateZeroOrPoison(sizeof(DrawData));
}

DrawCall::~DrawCall()
{
	sw::freeMemory(data);
}

Renderer::Renderer(vk::Device *device)
    : device(device)
{
	vertexProcessor.setRoutineCacheSize(1024);
	pixelProcessor.setRoutineCacheSize(1024);
	setupProcessor.setRoutineCacheSize(1024);
}

Renderer::~Renderer()
{
	drawTickets.take().wait();
}

// Renderer objects have to be mem aligned to the alignment provided in the class declaration
void *Renderer::operator new(size_t size)
{
	ASSERT(size == sizeof(Renderer));  // This operator can't be called from a derived class
	return vk::allocateHostMemory(sizeof(Renderer), alignof(Renderer), vk::NULL_ALLOCATION_CALLBACKS, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
}

void Renderer::operator delete(void *mem)
{
	vk::freeHostMemory(mem, vk::NULL_ALLOCATION_CALLBACKS);
}

void Renderer::draw(const vk::GraphicsPipeline *pipeline, const vk::DynamicState &dynamicState, unsigned int count, int baseVertex,
                    CountedEvent *events, int instanceID, int layer, void *indexBuffer, const VkRect2D &renderArea,
                    const vk::Pipeline::PushConstantStorage &pushConstants, bool update)
{
	if(count == 0) { return; }

	auto id = nextDrawID++;
	MARL_SCOPED_EVENT("draw %d", id);

	marl::Pool<sw::DrawCall>::Loan draw;
	{
		MARL_SCOPED_EVENT("drawCallPool.borrow()");
		draw = drawCallPool.borrow();
	}
	draw->id = id;

	const vk::GraphicsState &pipelineState = pipeline->getCombinedState(dynamicState);

	// A graphics pipeline must always be "complete" before it can be used for drawing.  A
	// complete graphics pipeline always includes the vertex input interface and
	// pre-rasterization subsets, but only includes fragment and fragment output interface
	// subsets if rasterizer discard is not enabled.
	//
	// Note that in the following, the setupPrimitives, setupRoutine and pixelRoutine functions
	// are only called when rasterizer discard is not enabled.  If rasterizer discard is
	// enabled, these functions and state for the latter two states are not set.
	const vk::VertexInputInterfaceState &vertexInputInterfaceState = pipelineState.getVertexInputInterfaceState();
	const vk::PreRasterizationState &preRasterizationState = pipelineState.getPreRasterizationState();
	const vk::FragmentState *fragmentState = nullptr;
	const vk::FragmentOutputInterfaceState *fragmentOutputInterfaceState = nullptr;

	const bool hasRasterizerDiscard = preRasterizationState.hasRasterizerDiscard();
	if(!hasRasterizerDiscard)
	{
		fragmentState = &pipelineState.getFragmentState();
		fragmentOutputInterfaceState = &pipelineState.getFragmentOutputInterfaceState();

		pixelProcessor.setBlendConstant(fragmentOutputInterfaceState->getBlendConstants());
	}

	const vk::Inputs &inputs = pipeline->getInputs();

	if(update)
	{
		MARL_SCOPED_EVENT("update");

		const sw::SpirvShader *fragmentShader = pipeline->getShader(VK_SHADER_STAGE_FRAGMENT_BIT).get();
		const sw::SpirvShader *vertexShader = pipeline->getShader(VK_SHADER_STAGE_VERTEX_BIT).get();

		const vk::Attachments attachments = pipeline->getAttachments();

		vertexState = vertexProcessor.update(pipelineState, vertexShader, inputs);
		vertexRoutine = vertexProcessor.routine(vertexState, preRasterizationState.getPipelineLayout(), vertexShader, inputs.getDescriptorSets());

		if(!hasRasterizerDiscard)
		{
			setupState = setupProcessor.update(pipelineState, fragmentShader, vertexShader, attachments);
			setupRoutine = setupProcessor.routine(setupState);

			pixelState = pixelProcessor.update(pipelineState, fragmentShader, vertexShader, attachments, hasOcclusionQuery());
			pixelRoutine = pixelProcessor.routine(pixelState, fragmentState->getPipelineLayout(), fragmentShader, attachments, inputs.getDescriptorSets());
		}
	}

	draw->preRasterizationContainsImageWrite = pipeline->preRasterizationContainsImageWrite();
	draw->fragmentContainsImageWrite = pipeline->fragmentContainsImageWrite();

	// The sample count affects the batch size even if rasterization is disabled.
	// TODO(b/147812380): Eliminate the dependency between multisampling and batch size.
	int ms = hasRasterizerDiscard ? 1 : fragmentOutputInterfaceState->getSampleCount();
	ASSERT(ms > 0);

	unsigned int numPrimitivesPerBatch = MaxBatchSize / ms;

	DrawData *data = draw->data;
	draw->occlusionQuery = occlusionQuery;
	draw->batchDataPool = &batchDataPool;
	draw->numPrimitives = count;
	draw->numPrimitivesPerBatch = numPrimitivesPerBatch;
	draw->numBatches = (count + draw->numPrimitivesPerBatch - 1) / draw->numPrimitivesPerBatch;
	draw->topology = vertexInputInterfaceState.getTopology();
	draw->provokingVertexMode = preRasterizationState.getProvokingVertexMode();
	draw->lineRasterizationMode = preRasterizationState.getLineRasterizationMode();
	draw->descriptorSetObjects = inputs.getDescriptorSetObjects();
	draw->preRasterizationPipelineLayout = preRasterizationState.getPipelineLayout();
	draw->depthClipEnable = preRasterizationState.getDepthClipEnable();
	draw->depthClipNegativeOneToOne = preRasterizationState.getDepthClipNegativeOneToOne();
	data->lineWidth = preRasterizationState.getLineWidth();
	data->rasterizerDiscard = hasRasterizerDiscard;

	data->descriptorSets = inputs.getDescriptorSets();
	data->descriptorDynamicOffsets = inputs.getDescriptorDynamicOffsets();

	for(int i = 0; i < MAX_INTERFACE_COMPONENTS / 4; i++)
	{
		const sw::Stream &stream = inputs.getStream(i);
		data->input[i] = stream.buffer;
		data->robustnessSize[i] = stream.robustnessSize;
		data->stride[i] = inputs.getVertexStride(i);
	}

	data->indices = indexBuffer;
	data->layer = layer;
	data->instanceID = instanceID;
	data->baseVertex = baseVertex;
	draw->indexType = indexBuffer ? pipeline->getIndexBuffer().getIndexType() : VK_INDEX_TYPE_UINT16;

	draw->vertexRoutine = vertexRoutine;

	vk::DescriptorSet::PrepareForSampling(draw->descriptorSetObjects, draw->preRasterizationPipelineLayout, device);

	// Viewport
	{
		const VkViewport &viewport = preRasterizationState.getViewport();

		float W = 0.5f * viewport.width;
		float H = 0.5f * viewport.height;
		float X0 = viewport.x + W;
		float Y0 = viewport.y + H;
		float N = viewport.minDepth;
		float F = viewport.maxDepth;
		float Z = F - N;
		constexpr float subPixF = vk::SUBPIXEL_PRECISION_FACTOR;

		data->WxF = W * subPixF;
		data->HxF = H * subPixF;
		data->X0xF = X0 * subPixF - subPixF / 2;
		data->Y0xF = Y0 * subPixF - subPixF / 2;
		data->halfPixelX = 0.5f / W;
		data->halfPixelY = 0.5f / H;
		data->depthRange = Z;
		data->depthNear = N;
		data->constantDepthBias = preRasterizationState.getConstantDepthBias();
		data->slopeDepthBias = preRasterizationState.getSlopeDepthBias();
		data->depthBiasClamp = preRasterizationState.getDepthBiasClamp();

		// Adjust viewport transform based on the negativeOneToOne state.
		if(preRasterizationState.getDepthClipNegativeOneToOne())
		{
			data->depthRange = Z * 0.5f;
			data->depthNear = (F + N) * 0.5f;
		}
	}

	// Scissor
	{
		const VkRect2D &scissor = preRasterizationState.getScissor();

		int x0 = renderArea.offset.x;
		int y0 = renderArea.offset.y;
		int x1 = x0 + renderArea.extent.width;
		int y1 = y0 + renderArea.extent.height;
		data->scissorX0 = clamp<int>(scissor.offset.x, x0, x1);
		data->scissorX1 = clamp<int>(scissor.offset.x + scissor.extent.width, x0, x1);
		data->scissorY0 = clamp<int>(scissor.offset.y, y0, y1);
		data->scissorY1 = clamp<int>(scissor.offset.y + scissor.extent.height, y0, y1);
	}

	if(!hasRasterizerDiscard)
	{
		const VkPolygonMode polygonMode = preRasterizationState.getPolygonMode();

		DrawCall::SetupFunction setupPrimitives = nullptr;
		if(vertexInputInterfaceState.isDrawTriangle(false, polygonMode))
		{
			switch(preRasterizationState.getPolygonMode())
			{
			case VK_POLYGON_MODE_FILL:
				setupPrimitives = &DrawCall::setupSolidTriangles;
				break;
			case VK_POLYGON_MODE_LINE:
				setupPrimitives = &DrawCall::setupWireframeTriangles;
				numPrimitivesPerBatch /= 3;
				break;
			case VK_POLYGON_MODE_POINT:
				setupPrimitives = &DrawCall::setupPointTriangles;
				numPrimitivesPerBatch /= 3;
				break;
			default:
				UNSUPPORTED("polygon mode: %d", int(preRasterizationState.getPolygonMode()));
				return;
			}
		}
		else if(vertexInputInterfaceState.isDrawLine(false, polygonMode))
		{
			setupPrimitives = &DrawCall::setupLines;
		}
		else  // Point primitive topology
		{
			setupPrimitives = &DrawCall::setupPoints;
		}

		draw->setupState = setupState;
		draw->setupRoutine = setupRoutine;
		draw->pixelRoutine = pixelRoutine;
		draw->setupPrimitives = setupPrimitives;
		draw->fragmentPipelineLayout = fragmentState->getPipelineLayout();

		if(pixelState.stencilActive)
		{
			data->stencil[0].set(fragmentState->getFrontStencil().reference, fragmentState->getFrontStencil().compareMask, fragmentState->getFrontStencil().writeMask);
			data->stencil[1].set(fragmentState->getBackStencil().reference, fragmentState->getBackStencil().compareMask, fragmentState->getBackStencil().writeMask);
		}

		data->factor = pixelProcessor.factor;

		if(pixelState.alphaToCoverage)
		{
			if(ms == 4)
			{
				data->a2c0 = 0.2f;
				data->a2c1 = 0.4f;
				data->a2c2 = 0.6f;
				data->a2c3 = 0.8f;
			}
			else if(ms == 2)
			{
				data->a2c0 = 0.25f;
				data->a2c1 = 0.75f;
			}
			else if(ms == 1)
			{
				data->a2c0 = 0.5f;
			}
			else
				ASSERT(false);
		}

		if(pixelState.occlusionEnabled)
		{
			for(int cluster = 0; cluster < MaxClusterCount; cluster++)
			{
				data->occlusion[cluster] = 0;
			}
		}

		// Viewport
		{
			const vk::Attachments attachments = pipeline->getAttachments();
			if(attachments.depthBuffer)
			{
				switch(attachments.depthBuffer->getFormat(VK_IMAGE_ASPECT_DEPTH_BIT))
				{
				case VK_FORMAT_D16_UNORM:
					// Minimum is 1 unit, but account for potential floating-point rounding errors
					data->minimumResolvableDepthDifference = 1.01f / 0xFFFF;
					break;
				case VK_FORMAT_D32_SFLOAT:
					// The minimum resolvable depth difference is determined per-polygon for floating-point depth
					// buffers. DrawData::minimumResolvableDepthDifference is unused.
					break;
				default:
					UNSUPPORTED("Depth format: %d", int(attachments.depthBuffer->getFormat(VK_IMAGE_ASPECT_DEPTH_BIT)));
				}
			}
		}

		// Target
		{
			const vk::Attachments attachments = pipeline->getAttachments();

			for(int index = 0; index < MAX_COLOR_BUFFERS; index++)
			{
				draw->colorBuffer[index] = attachments.colorBuffer[index];

				if(draw->colorBuffer[index])
				{
					data->colorBuffer[index] = (unsigned int *)attachments.colorBuffer[index]->getOffsetPointer({ 0, 0, 0 }, VK_IMAGE_ASPECT_COLOR_BIT, 0, data->layer);
					data->colorPitchB[index] = attachments.colorBuffer[index]->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
					data->colorSliceB[index] = attachments.colorBuffer[index]->slicePitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
				}
			}

			draw->depthBuffer = attachments.depthBuffer;
			draw->stencilBuffer = attachments.stencilBuffer;

			if(draw->depthBuffer)
			{
				data->depthBuffer = (float *)attachments.depthBuffer->getOffsetPointer({ 0, 0, 0 }, VK_IMAGE_ASPECT_DEPTH_BIT, 0, data->layer);
				data->depthPitchB = attachments.depthBuffer->rowPitchBytes(VK_IMAGE_ASPECT_DEPTH_BIT, 0);
				data->depthSliceB = attachments.depthBuffer->slicePitchBytes(VK_IMAGE_ASPECT_DEPTH_BIT, 0);
			}

			if(draw->stencilBuffer)
			{
				data->stencilBuffer = (unsigned char *)attachments.stencilBuffer->getOffsetPointer({ 0, 0, 0 }, VK_IMAGE_ASPECT_STENCIL_BIT, 0, data->layer);
				data->stencilPitchB = attachments.stencilBuffer->rowPitchBytes(VK_IMAGE_ASPECT_STENCIL_BIT, 0);
				data->stencilSliceB = attachments.stencilBuffer->slicePitchBytes(VK_IMAGE_ASPECT_STENCIL_BIT, 0);
			}
		}

		if(draw->fragmentPipelineLayout != draw->preRasterizationPipelineLayout)
		{
			vk::DescriptorSet::PrepareForSampling(draw->descriptorSetObjects, draw->fragmentPipelineLayout, device);
		}
	}

	// Push constants
	{
		data->pushConstants = pushConstants;
	}

	draw->events = events;

	DrawCall::run(device, draw, &drawTickets, clusterQueues);
}

void DrawCall::setup()
{
	if(occlusionQuery != nullptr)
	{
		occlusionQuery->start();
	}

	if(events)
	{
		events->add();
	}
}

void DrawCall::teardown(vk::Device *device)
{
	if(events)
	{
		events->done();
		events = nullptr;
	}

	vertexRoutine = {};
	setupRoutine = {};
	pixelRoutine = {};

	if(preRasterizationContainsImageWrite)
	{
		vk::DescriptorSet::ContentsChanged(descriptorSetObjects, preRasterizationPipelineLayout, device);
	}

	if(!data->rasterizerDiscard)
	{
		if(occlusionQuery != nullptr)
		{
			for(int cluster = 0; cluster < MaxClusterCount; cluster++)
			{
				occlusionQuery->add(data->occlusion[cluster]);
			}
			occlusionQuery->finish();
		}

		for(auto *target : colorBuffer)
		{
			if(target)
			{
				target->contentsChanged(vk::Image::DIRECT_MEMORY_ACCESS);
			}
		}

		// If pre-rasterization and fragment use the same pipeline, and pre-rasterization
		// also contains image writes, don't double-notify the descriptor set.
		const bool descSetAlreadyNotified = preRasterizationContainsImageWrite && fragmentPipelineLayout == preRasterizationPipelineLayout;
		if(fragmentContainsImageWrite && !descSetAlreadyNotified)
		{
			vk::DescriptorSet::ContentsChanged(descriptorSetObjects, fragmentPipelineLayout, device);
		}
	}
}

void DrawCall::run(vk::Device *device, const marl::Loan<DrawCall> &draw, marl::Ticket::Queue *tickets, marl::Ticket::Queue clusterQueues[MaxClusterCount])
{
	draw->setup();

	const auto numPrimitives = draw->numPrimitives;
	const auto numPrimitivesPerBatch = draw->numPrimitivesPerBatch;
	const auto numBatches = draw->numBatches;

	auto ticket = tickets->take();
	auto finally = marl::make_shared_finally([device, draw, ticket] {
		MARL_SCOPED_EVENT("FINISH draw %d", draw->id);
		draw->teardown(device);
		ticket.done();
	});

	for(unsigned int batchId = 0; batchId < numBatches; batchId++)
	{
		auto batch = draw->batchDataPool->borrow();
		batch->id = batchId;
		batch->firstPrimitive = batch->id * numPrimitivesPerBatch;
		batch->numPrimitives = std::min(batch->firstPrimitive + numPrimitivesPerBatch, numPrimitives) - batch->firstPrimitive;

		for(int cluster = 0; cluster < MaxClusterCount; cluster++)
		{
			batch->clusterTickets[cluster] = std::move(clusterQueues[cluster].take());
		}

		marl::schedule([device, draw, batch, finally] {
			processVertices(device, draw.get(), batch.get());

			if(!draw->data->rasterizerDiscard)
			{
				processPrimitives(device, draw.get(), batch.get());

				if(batch->numVisible > 0)
				{
					processPixels(device, draw, batch, finally);
					return;
				}
			}

			for(int cluster = 0; cluster < MaxClusterCount; cluster++)
			{
				batch->clusterTickets[cluster].done();
			}
		});
	}
}

void DrawCall::processVertices(vk::Device *device, DrawCall *draw, BatchData *batch)
{
	MARL_SCOPED_EVENT("VERTEX draw %d, batch %d", draw->id, batch->id);

	unsigned int triangleIndices[MaxBatchSize + 1][3];  // One extra for SIMD width overrun. TODO: Adjust to dynamic batch size.
	{
		MARL_SCOPED_EVENT("processPrimitiveVertices");
		processPrimitiveVertices(
		    triangleIndices,
		    draw->data->indices,
		    draw->indexType,
		    batch->firstPrimitive,
		    batch->numPrimitives,
		    draw->topology,
		    draw->provokingVertexMode);
	}

	auto &vertexTask = batch->vertexTask;
	vertexTask.primitiveStart = batch->firstPrimitive;
	// We're only using batch compaction for points, not lines
	vertexTask.vertexCount = batch->numPrimitives * ((draw->topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST) ? 1 : 3);
	if(vertexTask.vertexCache.drawCall != draw->id)
	{
		vertexTask.vertexCache.clear();
		vertexTask.vertexCache.drawCall = draw->id;
	}

	draw->vertexRoutine(device, &batch->triangles.front().v0, &triangleIndices[0][0], &vertexTask, draw->data);
}

void DrawCall::processPrimitives(vk::Device *device, DrawCall *draw, BatchData *batch)
{
	MARL_SCOPED_EVENT("PRIMITIVES draw %d batch %d", draw->id, batch->id);
	auto triangles = &batch->triangles[0];
	auto primitives = &batch->primitives[0];
	batch->numVisible = draw->setupPrimitives(device, triangles, primitives, draw, batch->numPrimitives);
}

void DrawCall::processPixels(vk::Device *device, const marl::Loan<DrawCall> &draw, const marl::Loan<BatchData> &batch, const std::shared_ptr<marl::Finally> &finally)
{
	struct Data
	{
		Data(const marl::Loan<DrawCall> &draw, const marl::Loan<BatchData> &batch, const std::shared_ptr<marl::Finally> &finally)
		    : draw(draw)
		    , batch(batch)
		    , finally(finally)
		{}
		marl::Loan<DrawCall> draw;
		marl::Loan<BatchData> batch;
		std::shared_ptr<marl::Finally> finally;
	};
	auto data = std::make_shared<Data>(draw, batch, finally);
	for(int cluster = 0; cluster < MaxClusterCount; cluster++)
	{
		batch->clusterTickets[cluster].onCall([device, data, cluster] {
			auto &draw = data->draw;
			auto &batch = data->batch;
			MARL_SCOPED_EVENT("PIXEL draw %d, batch %d, cluster %d", draw->id, batch->id, cluster);
			draw->pixelRoutine(device, &batch->primitives.front(), batch->numVisible, cluster, MaxClusterCount, draw->data);
			batch->clusterTickets[cluster].done();
		});
	}
}

void Renderer::synchronize()
{
	MARL_SCOPED_EVENT("synchronize");
	auto ticket = drawTickets.take();
	ticket.wait();
	device->updateSamplingRoutineSnapshotCache();
	ticket.done();
}

void DrawCall::processPrimitiveVertices(
    unsigned int triangleIndicesOut[MaxBatchSize + 1][3],
    const void *primitiveIndices,
    VkIndexType indexType,
    unsigned int start,
    unsigned int triangleCount,
    VkPrimitiveTopology topology,
    VkProvokingVertexModeEXT provokingVertexMode)
{
	if(!primitiveIndices)
	{
		struct LinearIndex
		{
			unsigned int operator[](unsigned int i) { return i; }
		};

		if(!setBatchIndices(triangleIndicesOut, topology, provokingVertexMode, LinearIndex(), start, triangleCount))
		{
			return;
		}
	}
	else
	{
		switch(indexType)
		{
		case VK_INDEX_TYPE_UINT8_EXT:
			if(!setBatchIndices(triangleIndicesOut, topology, provokingVertexMode, static_cast<const uint8_t *>(primitiveIndices), start, triangleCount))
			{
				return;
			}
			break;
		case VK_INDEX_TYPE_UINT16:
			if(!setBatchIndices(triangleIndicesOut, topology, provokingVertexMode, static_cast<const uint16_t *>(primitiveIndices), start, triangleCount))
			{
				return;
			}
			break;
		case VK_INDEX_TYPE_UINT32:
			if(!setBatchIndices(triangleIndicesOut, topology, provokingVertexMode, static_cast<const uint32_t *>(primitiveIndices), start, triangleCount))
			{
				return;
			}
			break;
			break;
		default:
			ASSERT(false);
			return;
		}
	}

	// setBatchIndices() takes care of the point case, since it's different due to the compaction
	if(topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
	{
		// Repeat the last index to allow for SIMD width overrun.
		triangleIndicesOut[triangleCount][0] = triangleIndicesOut[triangleCount - 1][2];
		triangleIndicesOut[triangleCount][1] = triangleIndicesOut[triangleCount - 1][2];
		triangleIndicesOut[triangleCount][2] = triangleIndicesOut[triangleCount - 1][2];
	}
}

int DrawCall::setupSolidTriangles(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count)
{
	auto &state = drawCall->setupState;

	int ms = state.multiSampleCount;
	const DrawData *data = drawCall->data;
	int visible = 0;

	for(int i = 0; i < count; i++, triangles++)
	{
		Vertex &v0 = triangles->v0;
		Vertex &v1 = triangles->v1;
		Vertex &v2 = triangles->v2;

		Polygon polygon(&v0.position, &v1.position, &v2.position);

		if((v0.cullMask | v1.cullMask | v2.cullMask) == 0)
		{
			continue;
		}

		if((v0.clipFlags & v1.clipFlags & v2.clipFlags) != Clipper::CLIP_FINITE)
		{
			continue;
		}

		int clipFlagsOr = v0.clipFlags | v1.clipFlags | v2.clipFlags;
		if(clipFlagsOr != Clipper::CLIP_FINITE)
		{
			if(!Clipper::Clip(polygon, clipFlagsOr, *drawCall))
			{
				continue;
			}
		}

		if(drawCall->setupRoutine(device, primitives, triangles, &polygon, data))
		{
			primitives += ms;
			visible++;
		}
	}

	return visible;
}

int DrawCall::setupWireframeTriangles(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count)
{
	auto &state = drawCall->setupState;

	int ms = state.multiSampleCount;
	int visible = 0;

	for(int i = 0; i < count; i++)
	{
		const Vertex &v0 = triangles[i].v0;
		const Vertex &v1 = triangles[i].v1;
		const Vertex &v2 = triangles[i].v2;

		float A = ((float)v0.projected.y - (float)v2.projected.y) * (float)v1.projected.x +
		          ((float)v2.projected.y - (float)v1.projected.y) * (float)v0.projected.x +
		          ((float)v1.projected.y - (float)v0.projected.y) * (float)v2.projected.x;  // Area

		int w0w1w2 = bit_cast<int>(v0.w) ^
		             bit_cast<int>(v1.w) ^
		             bit_cast<int>(v2.w);

		A = w0w1w2 < 0 ? -A : A;

		bool frontFacing = (state.frontFace == VK_FRONT_FACE_COUNTER_CLOCKWISE) ? (A >= 0.0f) : (A <= 0.0f);

		if(state.cullMode & VK_CULL_MODE_FRONT_BIT)
		{
			if(frontFacing) continue;
		}
		if(state.cullMode & VK_CULL_MODE_BACK_BIT)
		{
			if(!frontFacing) continue;
		}

		Triangle lines[3];
		lines[0].v0 = v0;
		lines[0].v1 = v1;
		lines[1].v0 = v1;
		lines[1].v1 = v2;
		lines[2].v0 = v2;
		lines[2].v1 = v0;

		for(int i = 0; i < 3; i++)
		{
			if(setupLine(device, *primitives, lines[i], *drawCall))
			{
				primitives += ms;
				visible++;
			}
		}
	}

	return visible;
}

int DrawCall::setupPointTriangles(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count)
{
	auto &state = drawCall->setupState;

	int ms = state.multiSampleCount;
	int visible = 0;

	for(int i = 0; i < count; i++)
	{
		const Vertex &v0 = triangles[i].v0;
		const Vertex &v1 = triangles[i].v1;
		const Vertex &v2 = triangles[i].v2;

		float d = (v0.y * v1.x - v0.x * v1.y) * v2.w +
		          (v0.x * v2.y - v0.y * v2.x) * v1.w +
		          (v2.x * v1.y - v1.x * v2.y) * v0.w;

		bool frontFacing = (state.frontFace == VK_FRONT_FACE_COUNTER_CLOCKWISE) ? (d > 0) : (d < 0);
		if(state.cullMode & VK_CULL_MODE_FRONT_BIT)
		{
			if(frontFacing) continue;
		}
		if(state.cullMode & VK_CULL_MODE_BACK_BIT)
		{
			if(!frontFacing) continue;
		}

		Triangle points[3];
		points[0].v0 = v0;
		points[1].v0 = v1;
		points[2].v0 = v2;

		for(int i = 0; i < 3; i++)
		{
			if(setupPoint(device, *primitives, points[i], *drawCall))
			{
				primitives += ms;
				visible++;
			}
		}
	}

	return visible;
}

int DrawCall::setupLines(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count)
{
	auto &state = drawCall->setupState;

	int visible = 0;
	int ms = state.multiSampleCount;

	for(int i = 0; i < count; i++)
	{
		if(setupLine(device, *primitives, *triangles, *drawCall))
		{
			primitives += ms;
			visible++;
		}

		triangles++;
	}

	return visible;
}

int DrawCall::setupPoints(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count)
{
	auto &state = drawCall->setupState;

	int visible = 0;
	int ms = state.multiSampleCount;

	for(int i = 0; i < count; i++)
	{
		if(setupPoint(device, *primitives, *triangles, *drawCall))
		{
			primitives += ms;
			visible++;
		}

		triangles++;
	}

	return visible;
}

bool DrawCall::setupLine(vk::Device *device, Primitive &primitive, Triangle &triangle, const DrawCall &draw)
{
	const Vertex &v0 = triangle.v0;
	const Vertex &v1 = triangle.v1;

	if((v0.cullMask | v1.cullMask) == 0)
	{
		return false;
	}

	const float4 &P0 = v0.position;
	const float4 &P1 = v1.position;

	if(P0.w <= 0 && P1.w <= 0)
	{
		return false;
	}

	const DrawData &data = *draw.data;
	const float lineWidth = data.lineWidth;
	const int clipFlags = draw.depthClipEnable ? Clipper::CLIP_FRUSTUM : Clipper::CLIP_SIDES;
	constexpr float subPixF = vk::SUBPIXEL_PRECISION_FACTOR;

	const float W = data.WxF * (1.0f / subPixF);
	const float H = data.HxF * (1.0f / subPixF);

	float dx = W * (P1.x / P1.w - P0.x / P0.w);
	float dy = H * (P1.y / P1.w - P0.y / P0.w);

	if(dx == 0 && dy == 0)
	{
		return false;
	}

	if(draw.lineRasterizationMode != VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT)
	{
		// Rectangle centered on the line segment

		float4 P[4];

		P[0] = P0;
		P[1] = P1;
		P[2] = P1;
		P[3] = P0;

		float scale = lineWidth * 0.5f / sqrt(dx * dx + dy * dy);

		dx *= scale;
		dy *= scale;

		float dx0h = dx * P0.w / H;
		float dy0w = dy * P0.w / W;

		float dx1h = dx * P1.w / H;
		float dy1w = dy * P1.w / W;

		P[0].x += -dy0w;
		P[0].y += +dx0h;

		P[1].x += -dy1w;
		P[1].y += +dx1h;

		P[2].x += +dy1w;
		P[2].y += -dx1h;

		P[3].x += +dy0w;
		P[3].y += -dx0h;

		Polygon polygon(P, 4);

		if(!Clipper::Clip(polygon, clipFlags, draw))
		{
			return false;
		}

		return draw.setupRoutine(device, &primitive, &triangle, &polygon, &data);
	}
	else if(false)  // TODO(b/80135519): Deprecate
	{
		// Connecting diamonds polygon
		// This shape satisfies the diamond test convention, except for the exit rule part.
		// Line segments with overlapping endpoints have duplicate fragments.
		// The ideal algorithm requires half-open line rasterization (b/80135519).

		float4 P[8];

		P[0] = P0;
		P[1] = P0;
		P[2] = P0;
		P[3] = P0;
		P[4] = P1;
		P[5] = P1;
		P[6] = P1;
		P[7] = P1;

		float dx0 = lineWidth * 0.5f * P0.w / W;
		float dy0 = lineWidth * 0.5f * P0.w / H;

		float dx1 = lineWidth * 0.5f * P1.w / W;
		float dy1 = lineWidth * 0.5f * P1.w / H;

		P[0].x += -dx0;
		P[1].y += +dy0;
		P[2].x += +dx0;
		P[3].y += -dy0;
		P[4].x += -dx1;
		P[5].y += +dy1;
		P[6].x += +dx1;
		P[7].y += -dy1;

		float4 L[6];

		if(dx > -dy)
		{
			if(dx > dy)  // Right
			{
				L[0] = P[0];
				L[1] = P[1];
				L[2] = P[5];
				L[3] = P[6];
				L[4] = P[7];
				L[5] = P[3];
			}
			else  // Down
			{
				L[0] = P[0];
				L[1] = P[4];
				L[2] = P[5];
				L[3] = P[6];
				L[4] = P[2];
				L[5] = P[3];
			}
		}
		else
		{
			if(dx > dy)  // Up
			{
				L[0] = P[0];
				L[1] = P[1];
				L[2] = P[2];
				L[3] = P[6];
				L[4] = P[7];
				L[5] = P[4];
			}
			else  // Left
			{
				L[0] = P[1];
				L[1] = P[2];
				L[2] = P[3];
				L[3] = P[7];
				L[4] = P[4];
				L[5] = P[5];
			}
		}

		Polygon polygon(L, 6);

		if(!Clipper::Clip(polygon, clipFlags, draw))
		{
			return false;
		}

		return draw.setupRoutine(device, &primitive, &triangle, &polygon, &data);
	}
	else
	{
		// Parallelogram approximating Bresenham line
		// This algorithm does not satisfy the ideal diamond-exit rule, but does avoid the
		// duplicate fragment rasterization problem and satisfies all of Vulkan's minimum
		// requirements for Bresenham line segment rasterization.

		float4 P[8];
		P[0] = P0;
		P[1] = P0;
		P[2] = P0;
		P[3] = P0;
		P[4] = P1;
		P[5] = P1;
		P[6] = P1;
		P[7] = P1;

		float dx0 = lineWidth * 0.5f * P0.w / W;
		float dy0 = lineWidth * 0.5f * P0.w / H;

		float dx1 = lineWidth * 0.5f * P1.w / W;
		float dy1 = lineWidth * 0.5f * P1.w / H;

		P[0].x += -dx0;
		P[1].y += +dy0;
		P[2].x += +dx0;
		P[3].y += -dy0;
		P[4].x += -dx1;
		P[5].y += +dy1;
		P[6].x += +dx1;
		P[7].y += -dy1;

		float4 L[4];

		if(dx > -dy)
		{
			if(dx > dy)  // Right
			{
				L[0] = P[1];
				L[1] = P[5];
				L[2] = P[7];
				L[3] = P[3];
			}
			else  // Down
			{
				L[0] = P[0];
				L[1] = P[4];
				L[2] = P[6];
				L[3] = P[2];
			}
		}
		else
		{
			if(dx > dy)  // Up
			{
				L[0] = P[0];
				L[1] = P[2];
				L[2] = P[6];
				L[3] = P[4];
			}
			else  // Left
			{
				L[0] = P[1];
				L[1] = P[3];
				L[2] = P[7];
				L[3] = P[5];
			}
		}

		Polygon polygon(L, 4);

		if(!Clipper::Clip(polygon, clipFlags, draw))
		{
			return false;
		}

		return draw.setupRoutine(device, &primitive, &triangle, &polygon, &data);
	}

	return false;
}

bool DrawCall::setupPoint(vk::Device *device, Primitive &primitive, Triangle &triangle, const DrawCall &draw)
{
	const Vertex &v = triangle.v0;

	if(v.cullMask == 0)
	{
		return false;
	}

	const DrawData &data = *draw.data;
	const int clipFlags = draw.depthClipEnable ? Clipper::CLIP_FRUSTUM : Clipper::CLIP_SIDES;

	const float pSize = clamp(v.pointSize, 1.0f, static_cast<float>(vk::MAX_POINT_SIZE));
	const float X = pSize * v.position.w * data.halfPixelX;
	const float Y = pSize * v.position.w * data.halfPixelY;

	float4 P[4];

	P[0] = v.position;
	P[0].x -= X;
	P[0].y += Y;

	P[1] = v.position;
	P[1].x += X;
	P[1].y += Y;

	P[2] = v.position;
	P[2].x += X;
	P[2].y -= Y;

	P[3] = v.position;
	P[3].x -= X;
	P[3].y -= Y;

	Polygon polygon(P, 4);

	if(!Clipper::Clip(polygon, clipFlags, draw))
	{
		return false;
	}

	primitive.pointSizeInv = 1.0f / pSize;

	return draw.setupRoutine(device, &primitive, &triangle, &polygon, &data);
}

void Renderer::addQuery(vk::Query *query)
{
	ASSERT(query->getType() == VK_QUERY_TYPE_OCCLUSION);
	ASSERT(!occlusionQuery);

	occlusionQuery = query;
}

void Renderer::removeQuery(vk::Query *query)
{
	ASSERT(query->getType() == VK_QUERY_TYPE_OCCLUSION);
	ASSERT(occlusionQuery == query);

	occlusionQuery = nullptr;
}

}  // namespace sw
