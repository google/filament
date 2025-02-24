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

#ifndef sw_Renderer_hpp
#define sw_Renderer_hpp

#include "PixelProcessor.hpp"
#include "Primitive.hpp"
#include "SetupProcessor.hpp"
#include "VertexProcessor.hpp"
#include "Vulkan/VkDescriptorSet.hpp"
#include "Vulkan/VkPipeline.hpp"

#include "marl/finally.h"
#include "marl/pool.h"
#include "marl/ticket.h"

#include <atomic>

namespace vk {

class DescriptorSet;
class Device;
class Query;
class PipelineLayout;

}  // namespace vk

namespace sw {

class CountedEvent;
struct DrawCall;
class PixelShader;
class VertexShader;
struct Task;
class Resource;
struct Constants;

static constexpr int MaxBatchSize = 128;
static constexpr int MaxBatchCount = 16;
static constexpr int MaxClusterCount = 16;
static constexpr int MaxDrawCount = 16;

using TriangleBatch = std::array<Triangle, MaxBatchSize>;
using PrimitiveBatch = std::array<Primitive, MaxBatchSize>;

struct DrawData
{
	vk::DescriptorSet::Bindings descriptorSets = {};
	vk::DescriptorSet::DynamicOffsets descriptorDynamicOffsets = {};

	const void *input[MAX_INTERFACE_COMPONENTS / 4];
	unsigned int robustnessSize[MAX_INTERFACE_COMPONENTS / 4];
	unsigned int stride[MAX_INTERFACE_COMPONENTS / 4];
	const void *indices;

	int instanceID;
	int baseVertex;
	float lineWidth;
	int layer;

	PixelProcessor::Stencil stencil[2];  // clockwise, counterclockwise
	PixelProcessor::Factor factor;
	unsigned int occlusion[MaxClusterCount];  // Number of pixels passing depth test

	float WxF;
	float HxF;
	float X0xF;
	float Y0xF;
	float halfPixelX;
	float halfPixelY;
	float depthRange;
	float depthNear;
	float minimumResolvableDepthDifference;
	float constantDepthBias;
	float slopeDepthBias;
	float depthBiasClamp;

	unsigned int *colorBuffer[MAX_COLOR_BUFFERS];
	int colorPitchB[MAX_COLOR_BUFFERS];
	int colorSliceB[MAX_COLOR_BUFFERS];
	float *depthBuffer;
	int depthPitchB;
	int depthSliceB;
	unsigned char *stencilBuffer;
	int stencilPitchB;
	int stencilSliceB;

	int scissorX0;
	int scissorX1;
	int scissorY0;
	int scissorY1;

	float a2c0;
	float a2c1;
	float a2c2;
	float a2c3;

	vk::Pipeline::PushConstantStorage pushConstants;

	bool rasterizerDiscard;
};

struct DrawCall
{
	struct BatchData
	{
		using Pool = marl::BoundedPool<BatchData, MaxBatchCount, marl::PoolPolicy::Preserve>;

		TriangleBatch triangles;
		PrimitiveBatch primitives;
		VertexTask vertexTask;
		unsigned int id;
		unsigned int firstPrimitive;
		unsigned int numPrimitives;
		int numVisible;
		marl::Ticket clusterTickets[MaxClusterCount];
	};

	using Pool = marl::BoundedPool<DrawCall, MaxDrawCount, marl::PoolPolicy::Preserve>;
	using SetupFunction = int (*)(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count);

	DrawCall();
	~DrawCall();

	static void run(vk::Device *device, const marl::Loan<DrawCall> &draw, marl::Ticket::Queue *tickets, marl::Ticket::Queue clusterQueues[MaxClusterCount]);
	static void processVertices(vk::Device *device, DrawCall *draw, BatchData *batch);
	static void processPrimitives(vk::Device *device, DrawCall *draw, BatchData *batch);
	static void processPixels(vk::Device *device, const marl::Loan<DrawCall> &draw, const marl::Loan<BatchData> &batch, const std::shared_ptr<marl::Finally> &finally);
	void setup();
	void teardown(vk::Device *device);

	int id;

	BatchData::Pool *batchDataPool;
	unsigned int numPrimitives;
	unsigned int numPrimitivesPerBatch;
	unsigned int numBatches;

	VkPrimitiveTopology topology;
	VkProvokingVertexModeEXT provokingVertexMode;
	VkIndexType indexType;
	VkLineRasterizationModeEXT lineRasterizationMode;

	bool depthClipEnable;
	bool depthClipNegativeOneToOne;

	VertexProcessor::RoutineType vertexRoutine;
	SetupProcessor::RoutineType setupRoutine;
	PixelProcessor::RoutineType pixelRoutine;
	bool preRasterizationContainsImageWrite;
	bool fragmentContainsImageWrite;

	SetupFunction setupPrimitives;
	SetupProcessor::State setupState;

	vk::ImageView *colorBuffer[MAX_COLOR_BUFFERS];
	vk::ImageView *depthBuffer;
	vk::ImageView *stencilBuffer;
	vk::DescriptorSet::Array descriptorSetObjects;
	const vk::PipelineLayout *preRasterizationPipelineLayout;
	const vk::PipelineLayout *fragmentPipelineLayout;
	sw::CountedEvent *events;

	vk::Query *occlusionQuery;

	DrawData *data;

	static void processPrimitiveVertices(
	    unsigned int triangleIndicesOut[MaxBatchSize + 1][3],
	    const void *primitiveIndices,
	    VkIndexType indexType,
	    unsigned int start,
	    unsigned int triangleCount,
	    VkPrimitiveTopology topology,
	    VkProvokingVertexModeEXT provokingVertexMode);

	static int setupSolidTriangles(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count);
	static int setupWireframeTriangles(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count);
	static int setupPointTriangles(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count);
	static int setupLines(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count);
	static int setupPoints(vk::Device *device, Triangle *triangles, Primitive *primitives, const DrawCall *drawCall, int count);

	static bool setupLine(vk::Device *device, Primitive &primitive, Triangle &triangle, const DrawCall &draw);
	static bool setupPoint(vk::Device *device, Primitive &primitive, Triangle &triangle, const DrawCall &draw);
};

class alignas(16) Renderer
{
public:
	Renderer(vk::Device *device);

	virtual ~Renderer();

	void *operator new(size_t size);
	void operator delete(void *mem);

	bool hasOcclusionQuery() const { return occlusionQuery != nullptr; }

	void draw(const vk::GraphicsPipeline *pipeline, const vk::DynamicState &dynamicState, unsigned int count, int baseVertex,
	          CountedEvent *events, int instanceID, int layer, void *indexBuffer, const VkRect2D &renderArea,
	          const vk::Pipeline::PushConstantStorage &pushConstants, bool update = true);

	void addQuery(vk::Query *query);
	void removeQuery(vk::Query *query);

	void synchronize();

private:
	DrawCall::Pool drawCallPool;
	DrawCall::BatchData::Pool batchDataPool;

	std::atomic<int> nextDrawID = { 0 };

	vk::Query *occlusionQuery = nullptr;
	marl::Ticket::Queue drawTickets;
	marl::Ticket::Queue clusterQueues[MaxClusterCount];

	VertexProcessor vertexProcessor;
	PixelProcessor pixelProcessor;
	SetupProcessor setupProcessor;

	VertexProcessor::State vertexState;
	SetupProcessor::State setupState;
	PixelProcessor::State pixelState;

	VertexProcessor::RoutineType vertexRoutine;
	SetupProcessor::RoutineType setupRoutine;
	PixelProcessor::RoutineType pixelRoutine;

	vk::Device *device;
};

}  // namespace sw

#endif  // sw_Renderer_hpp
