// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class QuerySetValidationTest : public ValidationTest {
  protected:
    wgpu::QuerySet CreateQuerySet(wgpu::Device cDevice,
                                  wgpu::QueryType queryType,
                                  uint32_t queryCount) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.type = queryType;
        descriptor.count = queryCount;

        return cDevice.CreateQuerySet(&descriptor);
    }
};

// Test creating query set without features
TEST_F(QuerySetValidationTest, CreationWithoutFeatures) {
    // Creating a query set for occlusion queries succeeds without any features enabled.
    CreateQuerySet(device, wgpu::QueryType::Occlusion, 1);
}

// Test creating query set with invalid count
TEST_F(QuerySetValidationTest, InvalidQueryCount) {
    // Success create a query set with the maximum count
    CreateQuerySet(device, wgpu::QueryType::Occlusion, kMaxQueryCount);

    // Fail to create a query set with the count which exceeds the maximum
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, wgpu::QueryType::Occlusion, kMaxQueryCount + 1));
}

// Test creating query set with invalid type
TEST_F(QuerySetValidationTest, InvalidQueryType) {
    ASSERT_DEVICE_ERROR(CreateQuerySet(device, static_cast<wgpu::QueryType>(0xFFFFFFFF), 1));
}

// Test destroying a destroyed query set
TEST_F(QuerySetValidationTest, DestroyDestroyedQuerySet) {
    wgpu::QuerySetDescriptor descriptor;
    descriptor.type = wgpu::QueryType::Occlusion;
    descriptor.count = 1;
    wgpu::QuerySet querySet = device.CreateQuerySet(&descriptor);
    querySet.Destroy();
    querySet.Destroy();
}

// Test that the query set creation parameters are correctly reflected for successfully created
// query sets.
TEST_F(QuerySetValidationTest, CreationParameterReflectionForValidQuerySet) {
    // Test reflection on two succesfully created but different query sets
    {
        wgpu::QuerySetDescriptor desc;
        desc.type = wgpu::QueryType::Occlusion;
        desc.count = 18;
        wgpu::QuerySet set = device.CreateQuerySet(&desc);

        EXPECT_EQ(wgpu::QueryType::Occlusion, set.GetType());
        EXPECT_EQ(18u, set.GetCount());
    }
    {
        wgpu::QuerySetDescriptor desc;
        // Unfortunately without extensions we can't check a different type.
        desc.type = wgpu::QueryType::Occlusion;
        desc.count = 1;
        wgpu::QuerySet set = device.CreateQuerySet(&desc);

        EXPECT_EQ(wgpu::QueryType::Occlusion, set.GetType());
        EXPECT_EQ(1u, set.GetCount());
    }
}

// Test that the query set creation parameters are correctly reflected for error query sets.
TEST_F(QuerySetValidationTest, CreationParameterReflectionForErrorQuerySet) {
    wgpu::QuerySetDescriptor desc;
    desc.type = static_cast<wgpu::QueryType>(0xFFFF);
    desc.count = 76;

    // Error! We have a garbage type.
    wgpu::QuerySet set;
    ASSERT_DEVICE_ERROR(set = device.CreateQuerySet(&desc));

    // Reflection data is still exactly what was in the descriptor.
    EXPECT_EQ(desc.type, set.GetType());
    EXPECT_EQ(76u, set.GetCount());
}

class OcclusionQueryValidationTest : public QuerySetValidationTest {};

// Test the occlusionQuerySet in RenderPassDescriptor
TEST_F(OcclusionQueryValidationTest, InvalidOcclusionQuerySet) {
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);
    PlaceholderRenderPass renderPass(device);

    // Success
    {
        renderPass.occlusionQuerySet = occlusionQuerySet;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.BeginOcclusionQuery(1);
        pass.EndOcclusionQuery();
        pass.End();
        encoder.Finish();
    }

    // Fail to begin occlusion query if the occlusionQuerySet is not set in RenderPassDescriptor
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass renderPassWithoutOcclusion(device);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassWithoutOcclusion);
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to begin render pass if the occlusionQuerySet is created from other device
    {
        wgpu::Device otherDevice = RequestDeviceSync(wgpu::DeviceDescriptor{});
        wgpu::QuerySet occlusionQuerySetOnOther =
            CreateQuerySet(otherDevice, wgpu::QueryType::Occlusion, 2);
        renderPass.occlusionQuerySet = occlusionQuerySetOnOther;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());

        // Clear this out so we don't hold a reference. The query set
        // must be destroyed before the device local to this test case.
        renderPass.occlusionQuerySet = wgpu::QuerySet();
    }

    // Fail to submit occlusion query with a destroyed query set
    {
        renderPass.occlusionQuerySet = occlusionQuerySet;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        wgpu::Queue queue = device.GetQueue();
        occlusionQuerySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test query index of occlusion query
TEST_F(OcclusionQueryValidationTest, InvalidQueryIndex) {
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);
    PlaceholderRenderPass renderPass(device);
    renderPass.occlusionQuerySet = occlusionQuerySet;

    // Fail to begin occlusion query if the query index exceeds the number of queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(2);
        pass.EndOcclusionQuery();
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Success to begin occlusion query with same query index twice on different render encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&renderPass);
        pass0.BeginOcclusionQuery(0);
        pass0.EndOcclusionQuery();
        pass0.End();

        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass);
        pass1.BeginOcclusionQuery(0);
        pass1.EndOcclusionQuery();
        pass1.End();
        encoder.Finish();
    }

    // Fail to begin occlusion query with same query index twice on a same render encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.BeginOcclusionQuery(0);
        pass.EndOcclusionQuery();
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test the correspondence between BeginOcclusionQuery and EndOcclusionQuery
TEST_F(OcclusionQueryValidationTest, InvalidBeginAndEnd) {
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);
    PlaceholderRenderPass renderPass(device);
    renderPass.occlusionQuerySet = occlusionQuerySet;

    // Fail to begin an occlusion query without corresponding end operation
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.BeginOcclusionQuery(1);
        pass.EndOcclusionQuery();
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to end occlusion query twice in a row even the begin occlusion query twice
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.BeginOcclusionQuery(0);
        pass.BeginOcclusionQuery(1);
        pass.EndOcclusionQuery();
        pass.EndOcclusionQuery();
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to end occlusion query without begin operation
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.EndOcclusionQuery();
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

class TimestampQueryValidationTest : public QuerySetValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::TimestampQuery};
    }

    void EncodeRenderPassWithTimestampWrites(wgpu::CommandEncoder encoder,
                                             const wgpu::PassTimestampWrites& timestampWrites) {
        PlaceholderRenderPass renderPass(device);
        renderPass.timestampWrites = &timestampWrites;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
    }

    void EncodeComputePassWithTimestampWrites(wgpu::CommandEncoder encoder,
                                              const wgpu::PassTimestampWrites& timestampWrites) {
        wgpu::ComputePassDescriptor descriptor;
        descriptor.timestampWrites = &timestampWrites;

        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&descriptor);
        pass.End();
    }
};

// Test creating query set with only the timestamp feature enabled.
TEST_F(TimestampQueryValidationTest, Creation) {
    // Creating a query set for occlusion queries succeeds.
    CreateQuerySet(device, wgpu::QueryType::Occlusion, 1);

    // Creating a query set for timestamp queries succeeds.
    CreateQuerySet(device, wgpu::QueryType::Timestamp, 1);
}

// Test query set with type of timestamp is set to the occlusionQuerySet of RenderPassDescriptor.
TEST_F(TimestampQueryValidationTest, SetOcclusionQueryWithTimestampQuerySet) {
    // Fail to begin render pass if the type of occlusionQuerySet is not Occlusion
    wgpu::QuerySet querySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 1);
    PlaceholderRenderPass renderPass(device);
    renderPass.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPass);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test timestampWrites in compute pass descriptor
TEST_F(TimestampQueryValidationTest, TimestampWritesOnComputePass) {
    wgpu::QuerySet querySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 2);

    // Success
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(encoder, {nullptr, querySet, 0, 1});
        encoder.Finish();
    }

    // Success when beginningOfPassWriteIndex is undefined.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(encoder,
                                             {nullptr, querySet, wgpu::kQuerySetIndexUndefined, 0});
        encoder.Finish();
    }

    // Success when endOfPassWriteIndex is undefined.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(encoder,
                                             {nullptr, querySet, 0, wgpu::kQuerySetIndexUndefined});
        encoder.Finish();
    }

    // Fail with struct chain.
    {
        wgpu::ChainedStruct chain = {};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(encoder, {&chain, querySet, 0, 1});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps when both beginningOfPassWriteIndex and endOfPassWriteIndex are
    // undefined.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(
            encoder,
            {nullptr, querySet, wgpu::kQuerySetIndexUndefined, wgpu::kQuerySetIndexUndefined});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps to other type of query set
    {
        wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 1);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(
            encoder, {nullptr, occlusionQuerySet, 0, wgpu::kQuerySetIndexUndefined});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps to the beginning of pass write index which exceeds the number of
    // queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(encoder,
                                             {nullptr, querySet, 2, wgpu::kQuerySetIndexUndefined});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps to the end of pass write index which exceeds the number of queries
    // in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(encoder,
                                             {nullptr, querySet, wgpu::kQuerySetIndexUndefined, 2});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Success to write timestamps to the same query index and location twice on different compute
    // pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(encoder, {nullptr, querySet, 0, 1});
        // Encodee other compute pass
        EncodeComputePassWithTimestampWrites(encoder, {nullptr, querySet, 0, 1});
        encoder.Finish();
    }

    // Fail to write timestamps to the same query index twice on same compute pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(encoder, {nullptr, querySet, 0, 0});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps to a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeComputePassWithTimestampWrites(encoder, {nullptr, querySet, 0, 1});

        wgpu::CommandBuffer commands = encoder.Finish();
        wgpu::Queue queue = device.GetQueue();
        querySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test timestampWrites in render pass descriptor
TEST_F(TimestampQueryValidationTest, TimestampWritesOnRenderPass) {
    wgpu::QuerySet querySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 2);

    // Success
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(encoder, {nullptr, querySet, 0, 1});
        encoder.Finish();
    }

    // Success when beginningOfPassWriteIndex is undefined.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(encoder,
                                            {nullptr, querySet, wgpu::kQuerySetIndexUndefined, 0});
        encoder.Finish();
    }

    // Success when endOfPassWriteIndex is undefined.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(encoder,
                                            {nullptr, querySet, 0, wgpu::kQuerySetIndexUndefined});
        encoder.Finish();
    }

    // Fail with struct chain.
    {
        wgpu::ChainedStruct chain = {};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(encoder, {&chain, querySet, 0, 1});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps when both beginningOfPassWriteIndex and endOfPassWriteIndex are
    // undefined.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(
            encoder,
            {nullptr, querySet, wgpu::kQuerySetIndexUndefined, wgpu::kQuerySetIndexUndefined});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps to other type of query set
    {
        wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 1);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(
            encoder, {nullptr, occlusionQuerySet, 0, wgpu::kQuerySetIndexUndefined});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps to the beginning of pass write index which exceeds the number of
    // queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(encoder,
                                            {nullptr, querySet, 2, wgpu::kQuerySetIndexUndefined});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps to the end of pass write index which exceeds the number of queries
    // in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(encoder,
                                            {nullptr, querySet, wgpu::kQuerySetIndexUndefined, 2});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Success to write timestamps to the same query index and location twice on different render
    // pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(encoder, {nullptr, querySet, 0, 1});
        // Encodee other render pass
        EncodeRenderPassWithTimestampWrites(encoder, {nullptr, querySet, 0, 1});
        encoder.Finish();
    }

    // Fail to write timestamps to the same query index twice on same render pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(encoder, {nullptr, querySet, 0, 0});
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamps to a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeRenderPassWithTimestampWrites(encoder, {nullptr, querySet, 0, 1});

        wgpu::CommandBuffer commands = encoder.Finish();
        wgpu::Queue queue = device.GetQueue();
        querySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test write timestamp on command encoder
TEST_F(TimestampQueryValidationTest, WriteTimestampOnCommandEncoder) {
    wgpu::QuerySet timestampQuerySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 2);
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);

    // Success on command encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(timestampQuerySet, 0);
        encoder.Finish();
    }

    // Fail to write timestamp to the index which exceeds the number of queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(timestampQuerySet, 2);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to submit timestamp query with a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(timestampQuerySet, 0);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetQueue();
        timestampQuerySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

class TimestampQueryInsidePassesValidationTest : public QuerySetValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        // The timestamp query feature must be supported if the chromium experimental timestamp
        // query inside passes feature is supported. Enable timestamp query for validating queries
        // overwrite inside and outside of the passes.
        return {wgpu::FeatureName::TimestampQuery,
                wgpu::FeatureName::ChromiumExperimentalTimestampQueryInsidePasses};
    }
};

// Test write timestamp on compute pass encoder
TEST_F(TimestampQueryInsidePassesValidationTest, WriteTimestampOnComputePassEncoder) {
    wgpu::QuerySet timestampQuerySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 2);
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);

    // Success on compute pass encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.End();
        encoder.Finish();
    }

    // Not allow to write timestamp to the query set with other query type
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(occlusionQuerySet, 0);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamp to the index which exceeds the number of queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(timestampQuerySet, 2);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to submit timestamp query with a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetQueue();
        timestampQuerySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test write timestamp on render pass encoder
TEST_F(TimestampQueryInsidePassesValidationTest, WriteTimestampOnRenderPassEncoder) {
    PlaceholderRenderPass renderPass(device);

    wgpu::QuerySet timestampQuerySet = CreateQuerySet(device, wgpu::QueryType::Timestamp, 2);
    wgpu::QuerySet occlusionQuerySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, 2);

    // Success on render pass encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.End();
        encoder.Finish();
    }

    // Not allow to write timestamp to the query set with other query type
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(occlusionQuerySet, 0);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to write timestamp to the index which exceeds the number of queries in query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 2);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Success to write timestamp to the same query index twice on command encoder and render
    // encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(timestampQuerySet, 0);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.End();
        encoder.Finish();
    }

    // Success to write timestamp to the same query index twice on different render encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&renderPass);
        pass0.WriteTimestamp(timestampQuerySet, 0);
        pass0.End();
        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPass);
        pass1.WriteTimestamp(timestampQuerySet, 0);
        pass1.End();
        encoder.Finish();
    }

    // Fail to write timestamp to the same query index twice on same render encoder
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to submit timestamp query with a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.WriteTimestamp(timestampQuerySet, 0);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetQueue();
        timestampQuerySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

class ResolveQuerySetValidationTest : public QuerySetValidationTest {
  protected:
    wgpu::Buffer CreateBuffer(wgpu::Device cDevice, uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;

        return cDevice.CreateBuffer(&descriptor);
    }
};

// Test resolve query set with invalid query set, first query and query count
TEST_F(ResolveQuerySetValidationTest, ResolveInvalidQuerySetAndIndexCount) {
    constexpr uint32_t kQueryCount = 4;

    wgpu::QuerySet querySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, kQueryCount);
    wgpu::Buffer destination =
        CreateBuffer(device, kQueryCount * sizeof(uint64_t), wgpu::BufferUsage::QueryResolve);

    // Success
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetQueue();
        queue.Submit(1, &commands);
    }

    //  Fail to resolve query set if first query out of range
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, kQueryCount, 0, destination, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set if the sum of first query and query count is larger than queries
    //  number in the query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 1, kQueryCount, destination, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to resolve a destroyed query set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetQueue();
        querySet.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test resolve query set with invalid query set, first query and query count
TEST_F(ResolveQuerySetValidationTest, ResolveToInvalidBufferAndOffset) {
    constexpr uint32_t kQueryCount = 4;
    constexpr uint64_t kBufferSize =
        (kQueryCount - 1) * sizeof(uint64_t) + kQueryResolveAlignment /*destinationOffset*/;

    wgpu::QuerySet querySet = CreateQuerySet(device, wgpu::QueryType::Occlusion, kQueryCount);
    wgpu::Buffer destination = CreateBuffer(device, kBufferSize, wgpu::BufferUsage::QueryResolve);

    // Success
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 1, kQueryCount - 1, destination, kQueryResolveAlignment);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetQueue();
        queue.Submit(1, &commands);
    }

    // Fail to resolve query set to a buffer created from another device
    {
        wgpu::Device otherDevice = RequestDeviceSync(wgpu::DeviceDescriptor{});
        wgpu::Buffer bufferOnOther =
            CreateBuffer(otherDevice, kBufferSize, wgpu::BufferUsage::QueryResolve);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, bufferOnOther, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set to a buffer if offset is not a multiple of 256 bytes
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, kQueryResolveAlignment / 2);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set to a buffer if the data size overflow the buffer
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, kQueryResolveAlignment);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set to a buffer if the offset is past the end of the buffer
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, 1, destination, kBufferSize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    //  Fail to resolve query set to a buffer does not have the usage of QueryResolve
    {
        wgpu::Buffer dstBuffer = CreateBuffer(device, kBufferSize, wgpu::BufferUsage::CopyDst);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, dstBuffer, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Fail to resolve query set to a destroyed buffer.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = device.GetQueue();
        destination.Destroy();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

}  // anonymous namespace
}  // namespace dawn
