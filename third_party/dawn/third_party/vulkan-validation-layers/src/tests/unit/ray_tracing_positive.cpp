/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"
#include "../framework/shader_helper.h"
#include "../framework/feature_requirements.h"
#include "../layers/utils/vk_layer_utils.h"
#include "../framework/descriptor_helper.h"
#include "../framework/pipeline_helper.h"
#include <iterator>

void RayTracingTest::InitFrameworkForRayTracingTest(VkValidationFeaturesEXT* enabled_features /*= nullptr*/) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(enabled_features));
}

class PositiveRayTracing : public RayTracingTest {};

TEST_F(PositiveRayTracing, GetAccelerationStructureBuildSizes) {
    TEST_DESCRIPTION("Test enabled features for GetAccelerationStructureBuildSizes");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkAccelerationStructureBuildGeometryInfoKHR build_info = vku::InitStructHelper();
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    uint32_t max_primitives_count = 0;
    VkAccelerationStructureBuildSizesInfoKHR build_sizes_info = vku::InitStructHelper();
    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR, &build_info,
                                              &max_primitives_count, &build_sizes_info);
}

TEST_F(PositiveRayTracing, AccelerationStructureReference) {
    TEST_DESCRIPTION("Test device side accelerationStructureReference");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();
    // Build Bottom Level Acceleration Structure
    auto blas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));
    blas->BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    m_command_buffer.Begin();
    // Build Top Level Acceleration Structure
    // ---
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, blas);
    tlas.BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveRayTracing, HostAccelerationStructureReference) {
    TEST_DESCRIPTION("Test host side accelerationStructureReference");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureHostCommands);
    RETURN_IF_SKIP(Init());

    // Build Bottom Level Acceleration Structure
    auto blas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device));
    blas->BuildHost();

    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostTopLevel(*m_device, blas);
    tlas.BuildHost();
}

TEST_F(PositiveRayTracing, CreateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.buffer = buffer.handle();
    as_create_info.size = 4096;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);
    vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
}

TEST_F(PositiveRayTracing, StridedDeviceAddressRegion) {
    TEST_DESCRIPTION("Test different valid VkStridedDeviceAddressRegionKHR");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline rt_pipeline(*this, m_device);

    rt_pipeline.SetGlslRayGenShader(kRayTracingMinimalGlsl);

    rt_pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    rt_pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    rt_pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    rt_pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    rt_pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    rt_pipeline.GetDescriptorSet().UpdateDescriptorSets();

    rt_pipeline.Build();

    vkt::rt::TraceRaysSbt sbt = rt_pipeline.GetTraceRaysSbt();

    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_pipeline.Handle());

    vk::CmdTraceRaysKHR(m_command_buffer.handle(), &sbt.ray_gen_sbt, &sbt.miss_sbt, &sbt.hit_sbt, &sbt.callable_sbt, 100, 100, 1);

    // pMissShaderBindingTable->deviceAddress == 0
    {
        VkStridedDeviceAddressRegionKHR null_addr_miss_sbt = sbt.miss_sbt;
        null_addr_miss_sbt.deviceAddress = 0;
        vk::CmdTraceRaysKHR(m_command_buffer.handle(), &sbt.ray_gen_sbt, &null_addr_miss_sbt, &sbt.hit_sbt, &sbt.callable_sbt, 100,
                            100, 1);
    }

    // pMissShaderBindingTable->size == 0 => region is considered unused so no error
    {
        VkStridedDeviceAddressRegionKHR null_addr_miss_sbt = sbt.miss_sbt;
        null_addr_miss_sbt.size = 0;
        null_addr_miss_sbt.stride = 0;
        vk::CmdTraceRaysKHR(m_command_buffer.handle(), &sbt.ray_gen_sbt, &null_addr_miss_sbt, &sbt.hit_sbt, &sbt.callable_sbt, 100,
                            100, 1);
    }

    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    m_device->Wait();
}

TEST_F(PositiveRayTracing, BarrierAccessMaskAccelerationStructureRayQueryEnabledRTXDisabled) {
    TEST_DESCRIPTION(
        "Test barrier with access ACCELERATION_STRUCTURE bit."
        "Ray query extension is enabled, as well as feature."
        "RTX extensions are disabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = 32;

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &buffer_barrier;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &image_barrier;

    m_command_buffer.Begin();

    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    m_command_buffer.End();
}

TEST_F(PositiveRayTracing, BarrierAccessMaskAccelerationStructureRayQueryEnabledRTXEnabled) {
    TEST_DESCRIPTION(
        "Test barrier with access ACCELERATION_STRUCTURE bit."
        "Ray query extension is enabled, as well as feature."
        "RTX extensions are enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(Init());

    VkMemoryBarrier2 mem_barrier = vku::InitStructHelper();
    mem_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkBufferMemoryBarrier2 buffer_barrier = vku::InitStructHelper();
    buffer_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    buffer_barrier.buffer = buffer.handle();
    buffer_barrier.size = 32;

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageMemoryBarrier2 image_barrier = vku::InitStructHelper();
    image_barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    image_barrier.image = image.handle();
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkDependencyInfo dependency_info = vku::InitStructHelper();
    dependency_info.memoryBarrierCount = 1;
    dependency_info.pMemoryBarriers = &mem_barrier;
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &buffer_barrier;
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &image_barrier;

    m_command_buffer.Begin();

    // specify VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR as srcStageMask and dstStageMask
    mem_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    buffer_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    image_barrier.srcAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    mem_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    buffer_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    image_barrier.dstAccessMask = VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependency_info);

    m_command_buffer.End();
}

TEST_F(PositiveRayTracing, BarrierSync1NoCrash) {
    TEST_DESCRIPTION("Regression test for nullptr crash when Sync1 barrier API is used for acceleration structure accesses");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // This stage can not be used with ACCELERATION_STRUCTURE_READ access when ray query is disabled, but VVL also should not crash.
    constexpr VkPipelineStageFlags invalid_src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    VkMemoryBarrier barrier = vku::InitStructHelper();
    barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

    m_errorMonitor->SetUnexpectedError("VUID-vkCmdPipelineBarrier-srcAccessMask-06257");
    m_command_buffer.Begin();
    vk::CmdPipelineBarrier(m_command_buffer.handle(), invalid_src_stage, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 1, &barrier, 0,
                           nullptr, 0, nullptr);
    m_command_buffer.End();
}

TEST_F(PositiveRayTracing, BuildAccelerationStructuresList) {
    TEST_DESCRIPTION("Build a list of destination acceleration structures, then do an update build on that same list");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    constexpr size_t blas_count = 10;

    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec;
    for (size_t i = 0; i < blas_count; ++i) {
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);
        blas_vec.emplace_back(std::move(blas));
    }

    m_command_buffer.Begin();
    vkt::as::BuildAccelerationStructuresKHR(m_command_buffer.handle(), blas_vec);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    for (auto& blas : blas_vec) {
        blas.SetSrcAS(blas.GetDstAS());
        blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
        blas.SetDstAS(vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, 4096));
    }

    m_command_buffer.Begin();
    vkt::as::BuildAccelerationStructuresKHR(m_command_buffer.handle(), blas_vec);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveRayTracing, BuildAccelerationStructuresList2) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, with first build having a bigger build range than second.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    VkPhysicalDeviceAccelerationStructurePropertiesKHR as_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 phys_dev_props = vku::InitStructHelper(&as_props);
    vk::GetPhysicalDeviceProperties2(m_device->Physical(), &phys_dev_props);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    auto scratch_buffer = std::make_shared<vkt::Buffer>(
        *m_device, 4 * 1024 * 1024, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);

    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec;

    auto blas_0 = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    std::vector<vkt::as::GeometryKHR> geometries;
    geometries.emplace_back(vkt::as::blueprint::GeometrySimpleOnDeviceTriangleInfo(*m_device, 1000));
    blas_0.SetGeometries(std::move(geometries));

    blas_0.SetScratchBuffer(scratch_buffer);

    auto blas_1 = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas_1.SetScratchBuffer(scratch_buffer);
    auto size_info_1 = blas_1.GetSizeInfo();

    // Scratch  buffer used ranges:
    // buffer start --> | blas_1 | <pad for alignment> | blas_0 |
    // If scratch size if computed incorrectly, an overlap with scratch memory for blas_0 will be detected for blas_1
    blas_0.SetDeviceScratchOffset(
        Align<VkDeviceAddress>(size_info_1.buildScratchSize, as_props.minAccelerationStructureScratchOffsetAlignment));

    blas_vec.emplace_back(std::move(blas_0));
    blas_vec.emplace_back(std::move(blas_1));

    m_command_buffer.Begin();

    vkt::as::BuildAccelerationStructuresKHR(m_command_buffer.handle(), blas_vec);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveRayTracing, AccelerationStructuresOverlappingMemory) {
    TEST_DESCRIPTION(
        "Validate acceleration structure building when source/destination acceleration structures and scratch buffers may "
        "overlap.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    constexpr size_t blas_count = 3;

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = (1u << 18) * blas_count;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    // Test using non overlapping memory chunks from the same buffer in multiple builds
    // The scratch buffer is used in multiple builds but bound at different offsets, so no validation error should be issued
    {
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.size = alloc_info.allocationSize;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        auto scratch_buffer = std::make_shared<vkt::Buffer>(*m_device, scratch_buffer_ci, vkt::no_mem);
        vk::BindBufferMemory(device(), scratch_buffer->handle(), buffer_memory.handle(), 0);
        std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec;
        VkDeviceSize consumed_buffer_size = 0;
        for (size_t i = 0; i < blas_count; ++i) {
            auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
            blas.SetScratchBuffer(scratch_buffer);
            blas.SetDeviceScratchOffset(consumed_buffer_size);
            consumed_buffer_size += blas.GetSizeInfo().buildScratchSize;
            consumed_buffer_size = Align<VkDeviceSize>(consumed_buffer_size, 4096);
            blas_vec.emplace_back(std::move(blas));
        }

        m_command_buffer.Begin();
        vkt::as::BuildAccelerationStructuresKHR(m_command_buffer.handle(), blas_vec);
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }
}

TEST_F(PositiveRayTracing, AccelerationStructuresReuseScratchMemory) {
    TEST_DESCRIPTION("Repro https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    // Allocate a memory chunk that will be used as backing memory for scratch buffer
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 1u << 18;
    vkt::DeviceMemory common_scratch_memory(*m_device, alloc_info);

    vkt::CommandBuffer cmd_buffer_frame_0(*m_device, m_command_pool);
    vkt::CommandBuffer cmd_buffer_frame_1(*m_device, m_command_pool);
    vkt::CommandBuffer cmd_buffer_frame_2(*m_device, m_command_pool);

    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_0;
    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_1;
    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_2;

    auto scratch_buffer_frame_0 = std::make_shared<vkt::Buffer>();
    auto scratch_buffer_frame_1 = std::make_shared<vkt::Buffer>();
    auto scratch_buffer_frame_2 = std::make_shared<vkt::Buffer>();

    vkt::Fence fence_frame_0(*m_device);
    vkt::Fence fence_frame_1(*m_device);
    vkt::Fence fence_frame_2(*m_device);

    // Frame 0
    {
        // Nothing to wait for, resources used in frame 0 will be released in frame 2

        // Create scratch buffer
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.size = alloc_info.allocationSize;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        scratch_buffer_frame_0->InitNoMemory(*m_device, scratch_buffer_ci);

        // Bind memory to scratch buffer
        vk::BindBufferMemory(device(), scratch_buffer_frame_0->handle(), common_scratch_memory.handle(), 0);

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetScratchBuffer(scratch_buffer_frame_0);
        blas_vec_frame_0.emplace_back(std::move(blas));
        cmd_buffer_frame_0.Begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_0.handle(), blas_vec_frame_0);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = scratch_buffer_frame_0->handle();
        barrier.size = scratch_buffer_ci.size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_0.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_0.End();
        m_default_queue->Submit(cmd_buffer_frame_0, fence_frame_0);
    }

    // Frame 1
    {
        // Still nothing to wait for

        // Create scratch buffer
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.size = alloc_info.allocationSize;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        scratch_buffer_frame_1->InitNoMemory(*m_device, scratch_buffer_ci);

        // Bind memory to scratch buffer
        vk::BindBufferMemory(device(), scratch_buffer_frame_1->handle(), common_scratch_memory.handle(), 0);

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetScratchBuffer(scratch_buffer_frame_1);
        blas_vec_frame_1.emplace_back(std::move(blas));
        cmd_buffer_frame_1.Begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_1.handle(), blas_vec_frame_1);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = scratch_buffer_frame_1->handle();
        barrier.size = scratch_buffer_ci.size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_1.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_1.End();
        m_default_queue->Submit(cmd_buffer_frame_1, fence_frame_1);
    }

    // Frame 2
    {
        // Free resources from frame 0
        fence_frame_0.Wait(kWaitTimeout);
        // Destroying buffer triggers VUID-vkDestroyBuffer-buffer-00922, it is still considered in use by cmd_buffer_frame_0 this
        // should not happen assuming synchronization is correct
        // Adding "fence_frame_1.Wait(kWaitTimeout);" used to solve this issue.
        // Using a dedicated memory chunk for each scratch buffer also used to solve it.
        // The issue was that when recording a acceleration structure build command,
        // any buffer indirectly mentioned through a device address used to be added using a call to GetBuffersByAddress.
        // So when recording the build happening on frame 1, given that all scratch buffers have the same base device address,
        // scratch_buffer_frame_0 was *also* be added as a child to cmd_buffer_frame_1.
        // So when destroying it hereinafter, since frame 1 is still in flight, scratch_buffer_frame_0 is still
        // considered in use, so 00922 is triggered.
        // => Solution: buffers obtained through a call to GetBuffersByAddress should not get added as children,
        // since there is no 1 to 1 mapping between a device address and a buffer.
        scratch_buffer_frame_0 = nullptr;  // Remove reference
        blas_vec_frame_0.clear();          // scratch_buffer_frame_0 will be destroyed in this call

        // Create scratch buffer
        VkBufferCreateInfo scratch_buffer_ci = vku::InitStructHelper();
        scratch_buffer_ci.size = alloc_info.allocationSize;
        scratch_buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        scratch_buffer_frame_2->InitNoMemory(*m_device, scratch_buffer_ci);

        // Bind memory to scratch buffer
        vk::BindBufferMemory(device(), scratch_buffer_frame_2->handle(), common_scratch_memory.handle(), 0);

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas.SetScratchBuffer(scratch_buffer_frame_2);
        blas_vec_frame_2.emplace_back(std::move(blas));
        cmd_buffer_frame_2.Begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_2.handle(), blas_vec_frame_2);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = scratch_buffer_frame_2->handle();
        barrier.size = scratch_buffer_ci.size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_2.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_2.End();
        m_default_queue->Submit(cmd_buffer_frame_2, fence_frame_2);
    }

    fence_frame_1.Wait(kWaitTimeout);
    fence_frame_2.Wait(kWaitTimeout);
}

TEST_F(PositiveRayTracing, AccelerationStructuresDedicatedScratchMemory) {
    TEST_DESCRIPTION(
        "Repro https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461"
        "This time, each scratch buffer has its own memory");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::CommandBuffer cmd_buffer_frame_0(*m_device, m_command_pool);
    vkt::CommandBuffer cmd_buffer_frame_1(*m_device, m_command_pool);
    vkt::CommandBuffer cmd_buffer_frame_2(*m_device, m_command_pool);

    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_0;
    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_1;
    std::vector<vkt::as::BuildGeometryInfoKHR> blas_vec_frame_2;

    vkt::Fence fence_frame_0(*m_device);
    vkt::Fence fence_frame_1(*m_device);
    vkt::Fence fence_frame_2(*m_device);

    // Frame 0
    {
        // Nothing to wait for, resources used in frame 0 will be released in frame 2

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);

        blas_vec_frame_0.emplace_back(std::move(blas));
        cmd_buffer_frame_0.Begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_0.handle(), blas_vec_frame_0);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = blas_vec_frame_0[0].GetScratchBuffer()->handle();
        barrier.size = blas_vec_frame_0[0].GetScratchBuffer()->CreateInfo().size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_0.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_0.End();
        m_default_queue->Submit(cmd_buffer_frame_0, fence_frame_0);
    }

    // Frame 1
    {
        // Still nothing to wait for

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_vec_frame_1.emplace_back(std::move(blas));
        cmd_buffer_frame_1.Begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_1.handle(), blas_vec_frame_1);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = blas_vec_frame_1[0].GetScratchBuffer()->handle();
        barrier.size = blas_vec_frame_1[0].GetScratchBuffer()->CreateInfo().size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_1.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_1.End();
        m_default_queue->Submit(cmd_buffer_frame_1, fence_frame_1);
    }

    // Frame 2
    {
        // Free resources from frame 0
        fence_frame_0.Wait(kWaitTimeout);
        blas_vec_frame_0.clear();  // No validation error

        // Build a dummy acceleration structure
        auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
        blas_vec_frame_2.emplace_back(std::move(blas));
        cmd_buffer_frame_2.Begin();
        vkt::as::BuildAccelerationStructuresKHR(cmd_buffer_frame_2.handle(), blas_vec_frame_2);

        // Synchronize accesses to scratch buffer memory: next op will be a new acceleration structure build
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = blas_vec_frame_2[0].GetScratchBuffer()->handle();
        barrier.size = blas_vec_frame_2[0].GetScratchBuffer()->CreateInfo().size;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vk::CmdPipelineBarrier(cmd_buffer_frame_2.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        cmd_buffer_frame_2.End();
        m_default_queue->Submit(cmd_buffer_frame_2, fence_frame_2);
    }

    fence_frame_1.Wait(kWaitTimeout);
    fence_frame_2.Wait(kWaitTimeout);
}

TEST_F(PositiveRayTracing, CmdBuildAccelerationStructuresIndirect) {
    TEST_DESCRIPTION("basic usage of vkCmdBuildAccelerationStructuresIndirectKHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureIndirectBuild);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    m_command_buffer.Begin();
    blas.BuildCmdBufferIndirect(m_command_buffer.handle());
    m_command_buffer.End();
}

TEST_F(PositiveRayTracing, ScratchBufferCorrectAddressSpaceOpBuild) {
    TEST_DESCRIPTION(
        "Have two scratch buffers bound to the same memory, with one of them being not big enough for an acceleration structure "
        "build, but the other one is. If the buffer addresses of those buffers are the same, 03671 should not fire");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    auto size_info = blas.GetSizeInfo();
    if (size_info.buildScratchSize <= 64) {
        GTEST_SKIP() << "Need a big scratch size, skipping test.";
    }

    VkPhysicalDeviceAccelerationStructurePropertiesKHR acc_struct_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(acc_struct_properties);
    VkDeviceSize scratch_size = size_info.buildScratchSize + acc_struct_properties.minAccelerationStructureScratchOffsetAlignment;
    scratch_size = Align<VkDeviceSize>(scratch_size, acc_struct_properties.minAccelerationStructureScratchOffsetAlignment);

    // Allocate buffer memory separately so that it can be large enough. Scratch buffer size will be smaller.
    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = scratch_size;
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);

    VkBufferCreateInfo small_buffer_ci = vku::InitStructHelper();
    small_buffer_ci.size = 64;
    small_buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    auto small_scratch_buffer = std::make_shared<vkt::Buffer>(*m_device, small_buffer_ci, vkt::no_mem);
    small_scratch_buffer->BindMemory(buffer_memory, 0);

    small_buffer_ci.size = alloc_info.allocationSize;
    auto big_scratch_buffer = std::make_shared<vkt::Buffer>(*m_device, small_buffer_ci, vkt::no_mem);
    big_scratch_buffer->BindMemory(buffer_memory, 0);
    const VkDeviceAddress big_scratch_address = big_scratch_buffer->Address();
    if (big_scratch_address != small_scratch_buffer->Address()) {
        GTEST_SKIP() << "Binding two buffers to the same memory does not yield identical buffer addresses, skipping test.";
    }

    m_command_buffer.Begin();
    blas.SetScratchBuffer(small_scratch_buffer);
    blas.BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveRayTracing, BasicTraceRays) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders) and acceleration structure, and trace one "
        "ray. Only call traceRay in the ray generation shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    // Build pipeline
    pipeline.Build();

    // Bind descriptor set, pipeline, and trace rays
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveRayTracing, BasicTraceRaysDeferredBuild) {
    TEST_DESCRIPTION(
        "Setup a ray tracing pipeline (ray generation, miss and closest hit shaders, and deferred build) and acceleration "
        "structure, and trace one "
        "ray. Only call traceRay in the ray generation shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
        traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
        )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
        hit = vec3(0.1, 0.2, 0.3);
        }
        )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
        const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
        hit = barycentricCoords;
        }
        )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    // Deferred pipeline build
    RETURN_IF_SKIP(pipeline.DeferBuild());
    RETURN_IF_SKIP(pipeline.Build());

    // Bind descriptor set, pipeline, and trace rays
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveRayTracing, GetAccelerationStructureAddressBadBuffer) {
    TEST_DESCRIPTION(
        "Call vkGetAccelerationStructureDeviceAddressKHR on an acceleration structure whose buffer is missing usage "
        "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, and whose memory has been destroyed");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage);
    buffer_ci.size = 4096;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(&alloc_flags);
    alloc_info.allocationSize = 4096;
    vkt::DeviceMemory mem(*m_device, alloc_info);
    vk::BindBufferMemory(device(), buffer.handle(), mem.handle(), 0);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR as_create_info = vku::InitStructHelper();
    as_create_info.buffer = buffer.handle();
    as_create_info.size = 4096;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    vk::CreateAccelerationStructureKHR(device(), &as_create_info, nullptr, &as);

    VkAccelerationStructureDeviceAddressInfoKHR as_address_info = vku::InitStructHelper();
    as_address_info.accelerationStructure = as;
    vk::GetAccelerationStructureDeviceAddressKHR(device(), &as_address_info);
    vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
}

// Use to be invalid, but VUID-vkCmdBuildAccelerationStructuresKHR-firstVertex-03770 was removed in
// https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6733
TEST_F(PositiveRayTracing, UpdatedFirstVertex) {
    TEST_DESCRIPTION(
        "Build a list of destination acceleration structures, then do an update build on that same list but with a different "
        "VkAccelerationStructureBuildRangeInfoKHR::firstVertex");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    m_command_buffer.Begin();
    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    blas.BuildCmdBuffer(m_command_buffer.handle());

    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    m_command_buffer.Begin();

    blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);
    blas.SetSrcAS(blas.GetDstAS());

    // Create custom build ranges, with the default valid as a template, then somehow supply it?
    auto build_range_infos = blas.GetBuildRangeInfosFromGeometries();
    build_range_infos[0].firstVertex = 666;
    blas.SetBuildRanges(build_range_infos);

    blas.BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.End();
}

TEST_F(PositiveRayTracing, BindGraphicsPipelineAfterRayTracingPipeline) {
    TEST_DESCRIPTION("Bind a graphics pipeline width dynamic line width state after binding ray tracing pipeline");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    vkt::rt::Pipeline pipeline(*this, m_device);

    // Set shaders

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require

        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    // Build pipeline
    pipeline.Build();

    CreatePipelineHelper graphics_pipeline(*this);
    graphics_pipeline.AddDynamicState(VK_DYNAMIC_STATE_LINE_WIDTH);
    graphics_pipeline.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    graphics_pipeline.CreateGraphicsPipeline();

    // Bind descriptor set, pipeline, and trace rays
    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.Handle());
    vk::CmdSetLineWidth(m_command_buffer, 1.0f);
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveRayTracing, InstanceBufferBadAddress) {
    TEST_DESCRIPTION("Use an invalid address for an instance buffer, but also specify a primitiveCount of 0 => no errors");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    auto blas =
        std::make_shared<vkt::as::BuildGeometryInfoKHR>(vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device));

    m_command_buffer.Begin();
    blas->BuildCmdBuffer(m_command_buffer);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    auto tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, blas);

    m_command_buffer.Begin();
    tlas.SetupBuild(*m_device, true);

    auto build_range_infos = tlas.GetBuildRangeInfosFromGeometries();
    build_range_infos[0].primitiveCount = 0;
    tlas.SetBuildRanges(build_range_infos);

    tlas.GetGeometries()[0].SetInstancesDeviceAddress(0);

    tlas.VkCmdBuildAccelerationStructuresKHR(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(PositiveRayTracing, WriteAccelerationStructuresPropertiesDevice) {
    TEST_DESCRIPTION("Test getting query results from vkCmdWriteAccelerationStructuresPropertiesKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingMaintenance1);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 4 * sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    vkt::as::BuildGeometryInfoKHR blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, 1);

    m_command_buffer.Begin();
    blas.BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0u, 1u);
    vk::CmdWriteAccelerationStructuresPropertiesKHR(m_command_buffer.handle(), 1, &blas.GetDstAS()->handle(),
                                                    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, query_pool.handle(), 0);
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0u, 1u, buffer, 0u, sizeof(uint64_t),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}

TEST_F(PositiveRayTracing, BasicOpacityMicromapBuild) {
    TEST_DESCRIPTION("Test building an opacity micromap then building an acceleration structure with that");

    // Mask data for 2 levels of subdivision. Middle triangle is index 1, so drop that one out.
    // Bit string for middle missing is '1011' (0 on the left). In number form, that's 0xd.
    // Extending the Sierpinski-esque pattern out one level is 0xdd0d
    uint32_t testMask = 0xdd0d;

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);

    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::synchronization2);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::micromap);
    AddRequiredFeature(vkt::Feature::micromapHostCommands);

    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    VkMemoryAllocateFlagsInfo allocate_da_flag_info = vku::InitStructHelper();
    allocate_da_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;


    // Create a buffer with the mask and index data
    vkt::Buffer micromapDataBuffer(*m_device, 2*1048576 /*XXX*/,
        VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        vkt::device_address);

    VkDeviceAddress micromapAddress = micromapDataBuffer.Address();

    // Fill out VkMicromapUsageEXT with size information
    VkMicromapUsageEXT mmUsage = { };
    mmUsage.count = 1;

    const int TriangleOffset = 0;
    const int IndexOffset = 256;
    const int DataOffset = 512;

    mmUsage.subdivisionLevel = 2;
    mmUsage.format = VK_OPACITY_MICROMAP_FORMAT_2_STATE_EXT;

    {
        uint32_t* data = (uint32_t*)micromapDataBuffer.Memory().Map();

        VkMicromapTriangleEXT* tri = (VkMicromapTriangleEXT*)&data[TriangleOffset/4];
        tri->dataOffset = 0;
        tri->subdivisionLevel = uint16_t(mmUsage.subdivisionLevel);
        tri->format = uint16_t(mmUsage.format);

        // Micromap data
        // Just replicate for testing higher subdivision
        {
            uint32_t maskWord = testMask | (testMask << 16);
            int words = ((1 << (2*mmUsage.subdivisionLevel)) + 31) / 32;
            for (int i = 0; i<words; i++) {
                data[DataOffset / 4 + i] = maskWord;
            }
        }

        // Index information
        data[IndexOffset/4] = 0;

        micromapDataBuffer.Memory().Unmap();
    }

    VkMicromapBuildInfoEXT mmBuildInfo = { VK_STRUCTURE_TYPE_MICROMAP_BUILD_INFO_EXT };

    mmBuildInfo.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    mmBuildInfo.flags = 0;
    mmBuildInfo.mode = VK_BUILD_MICROMAP_MODE_BUILD_EXT;
    mmBuildInfo.dstMicromap = VK_NULL_HANDLE;
    mmBuildInfo.usageCountsCount = 1;
    mmBuildInfo.pUsageCounts = &mmUsage;
    mmBuildInfo.data.deviceAddress = 0ull;
    mmBuildInfo.triangleArray.deviceAddress = 0ull;
    mmBuildInfo.triangleArrayStride = 0;

    VkMicromapBuildSizesInfoEXT sizeInfo = { VK_STRUCTURE_TYPE_MICROMAP_BUILD_SIZES_INFO_EXT };

    vk::GetMicromapBuildSizesEXT(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &mmBuildInfo, &sizeInfo);

    // Create a buffer and micromap on top from the size
    vkt::Buffer micromapBuffer(*m_device, sizeInfo.micromapSize, VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT);
    // Scratch buffer
    vkt::Buffer msBuffer(*m_device, sizeInfo.buildScratchSize > 4 ? sizeInfo.buildScratchSize : 4,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &allocate_da_flag_info);

    VkDeviceAddress msAddress = msBuffer.Address();

    VkMicromapEXT micromap;

    VkMicromapCreateInfoEXT maCreateInfo = { VK_STRUCTURE_TYPE_MICROMAP_CREATE_INFO_EXT };

    maCreateInfo.createFlags = 0;
    maCreateInfo.buffer = micromapBuffer;
    maCreateInfo.offset = 0;
    maCreateInfo.size = sizeInfo.micromapSize;
    maCreateInfo.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    maCreateInfo.deviceAddress = 0ull;

    VkResult result = vk::CreateMicromapEXT(device(), &maCreateInfo, nullptr, &micromap);
    ASSERT_EQ(VK_SUCCESS, result);

    // Build the array with vkBuildmicromapsEXT
    {
        // Fill in the pointers we didn't have at size query
        mmBuildInfo.dstMicromap = micromap;
        mmBuildInfo.data.deviceAddress = micromapAddress+DataOffset;
        mmBuildInfo.triangleArray.deviceAddress = micromapAddress+TriangleOffset;
        mmBuildInfo.scratchData.deviceAddress = msAddress;

        m_command_buffer.Begin();

        vk::CmdBuildMicromapsEXT(m_command_buffer.handle(), 1, &mmBuildInfo);

        {
            VkMemoryBarrier2 memoryBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2, NULL,
                VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT, VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT,
                VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_MICROMAP_READ_BIT_EXT };
            VkDependencyInfo dependencyInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };

            dependencyInfo.memoryBarrierCount = 1;
            dependencyInfo.pMemoryBarriers = &memoryBarrier;

            vk::CmdPipelineBarrier2KHR(m_command_buffer.handle(), &dependencyInfo);
        }
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }

    // Create a buffer with the triangle data in it
    static float const vertexData[6*2] = {
        0.25, 0.75,
        0.5, 0.25,
        0.75, 0.75,
    };
    static uint32_t const indexData[6] = { 0, 1, 2 };

    vkt::Buffer vertexBuffer(*m_device, sizeof(vertexData) + sizeof(indexData),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);

    VkDeviceAddress vertexAddress = vertexBuffer.Address();

    // Upload data to the vertex buffer.
    {
        char* ptr;

        vk::MapMemory(device(), vertexBuffer.Memory(), 0, VK_WHOLE_SIZE, 0, (void**)&ptr);

        memcpy(ptr, &vertexData[0], sizeof(vertexData));
        memcpy(ptr+sizeof(vertexData), &indexData[0], sizeof(indexData));

        vk::UnmapMemory(device(), vertexBuffer.Memory());
    }

    VkAccelerationStructureBuildSizesInfoKHR bottomASBuildSizesInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
    VkAccelerationStructureBuildSizesInfoKHR topASBuildSizesInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};

    // Create a bottom-level acceleration structure with one triangle
    VkAccelerationStructureGeometryKHR bottomASGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };

    bottomASGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    bottomASGeometry.geometry.triangles = vku::InitStructHelper();
    bottomASGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32_SFLOAT;
    bottomASGeometry.geometry.triangles.vertexData.deviceAddress = vertexAddress;
    bottomASGeometry.geometry.triangles.vertexStride = 8;
    bottomASGeometry.geometry.triangles.maxVertex = 3;
    bottomASGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    bottomASGeometry.geometry.triangles.indexData.deviceAddress = vertexAddress + sizeof(vertexData);
    bottomASGeometry.geometry.triangles.transformData.deviceAddress = 0;
    bottomASGeometry.flags = 0;

    VkAccelerationStructureTrianglesOpacityMicromapEXT opacityGeometryMicromap = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT };

    opacityGeometryMicromap.indexType = VK_INDEX_TYPE_UINT32;
    opacityGeometryMicromap.indexBuffer.deviceAddress = micromapAddress + IndexOffset;
    opacityGeometryMicromap.indexStride = 0;
    opacityGeometryMicromap.baseTriangle = 0;
    opacityGeometryMicromap.micromap = micromap;
    bottomASGeometry.geometry.triangles.pNext = &opacityGeometryMicromap;

    VkAccelerationStructureBuildGeometryInfoKHR bottomASInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
    bottomASInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    bottomASInfo.flags = 0;
    bottomASInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    bottomASInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    bottomASInfo.dstAccelerationStructure = VK_NULL_HANDLE;
    bottomASInfo.geometryCount = 1;
    bottomASInfo.pGeometries = &bottomASGeometry;
    bottomASInfo.ppGeometries = NULL;
    bottomASInfo.scratchData.deviceAddress = 0;

    uint32_t bottomMaxPrimitiveCounts = 1;

    vk::GetAccelerationStructureBuildSizesKHR(*m_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &bottomASInfo, &bottomMaxPrimitiveCounts, &bottomASBuildSizesInfo);

    vkt::Buffer bottomASBuffer(*m_device, bottomASBuildSizesInfo.accelerationStructureSize,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &allocate_da_flag_info);

    VkAccelerationStructureCreateInfoKHR asCreateInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };

    asCreateInfo.createFlags = 0;
    asCreateInfo.buffer = bottomASBuffer;
    asCreateInfo.offset = 0;
    asCreateInfo.size = bottomASBuildSizesInfo.accelerationStructureSize;
    asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    asCreateInfo.deviceAddress = 0;

    VkAccelerationStructureKHR bottomAS, topAS;

    result = vk::CreateAccelerationStructureKHR(*m_device, &asCreateInfo, NULL, &bottomAS);
    ASSERT_EQ(VK_SUCCESS, result);

    vkt::Buffer instanceBuffer(*m_device, 2 * sizeof(VkAccelerationStructureInstanceKHR),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vkt::device_address);

    VkDeviceAddress instanceAddress = instanceBuffer.Address();

    {
        VkAccelerationStructureInstanceKHR* instance = (VkAccelerationStructureInstanceKHR*)instanceBuffer.Memory().Map();

        memset(instance, 0, 2 * sizeof(VkAccelerationStructureInstanceKHR));

        instance[0].transform.matrix[0][0] = 1;
        instance[0].transform.matrix[0][1] = 0;
        instance[0].transform.matrix[0][2] = 0;
        instance[0].transform.matrix[0][3] = 0;

        instance[0].transform.matrix[1][0] = 0;
        instance[0].transform.matrix[1][1] = 1;
        instance[0].transform.matrix[1][2] = 0;
        instance[0].transform.matrix[1][3] = 0;

        instance[0].transform.matrix[2][0] = 0;
        instance[0].transform.matrix[2][1] = 0;
        instance[0].transform.matrix[2][2] = 1;
        instance[0].transform.matrix[2][3] = 0;

        instance[0].instanceCustomIndex = 0xdeadfe;
        instance[0].mask = 0xff;
        instance[0].instanceShaderBindingTableRecordOffset = 0;
        instance[0].flags = 0;

        VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddressInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
        asDeviceAddressInfo.accelerationStructure = bottomAS;
        instance[0].accelerationStructureReference =  vk::GetAccelerationStructureDeviceAddressKHR(device(), &asDeviceAddressInfo);

        instanceBuffer.Memory().Unmap();
    }

    VkAccelerationStructureGeometryKHR topASGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };

    topASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    topASGeometry.geometry.instances = vku::InitStructHelper();
    topASGeometry.geometry.instances.pNext = NULL;
    topASGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
    topASGeometry.geometry.instances.data.deviceAddress = instanceAddress;
    topASGeometry.flags = 0;

    VkAccelerationStructureBuildGeometryInfoKHR topASInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
    topASInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    topASInfo.flags = 0;
    topASInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    topASInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    topASInfo.dstAccelerationStructure = VK_NULL_HANDLE;
    topASInfo.geometryCount = 1;
    topASInfo.pGeometries = &topASGeometry;
    topASInfo.ppGeometries = NULL;
    topASInfo.scratchData.deviceAddress = 0;

    uint32_t topMaxPrimitiveCounts = 1;

    vk::GetAccelerationStructureBuildSizesKHR(device(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &topASInfo, &topMaxPrimitiveCounts, &topASBuildSizesInfo);

    vkt::Buffer topASBuffer(*m_device, topASBuildSizesInfo.accelerationStructureSize,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &allocate_da_flag_info);


    asCreateInfo.createFlags = 0;
    asCreateInfo.buffer = topASBuffer;
    asCreateInfo.offset = 0;
    asCreateInfo.size = topASBuildSizesInfo.accelerationStructureSize;
    asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    asCreateInfo.deviceAddress = 0;

    result = vk::CreateAccelerationStructureKHR(device(), &asCreateInfo, NULL, &topAS);
    ASSERT_EQ(VK_SUCCESS, result);

    vkt::Buffer scratchBuffer(*m_device, std::max(bottomASBuildSizesInfo.buildScratchSize, topASBuildSizesInfo.buildScratchSize),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &allocate_da_flag_info);

    VkDeviceAddress scratchAddress = scratchBuffer.Address();

    {
        bottomASInfo.dstAccelerationStructure = bottomAS;
        bottomASInfo.scratchData.deviceAddress = scratchAddress;

        VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
            1, 0, 0, 0,
        };

        const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfo = &buildRangeInfo;

        // Build the bottom-level acceleration structure
        m_command_buffer.Begin();

        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer.handle(), 1, &bottomASInfo, &pBuildRangeInfo);
        VkMemoryBarrier memoryBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER, NULL,
            VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR };
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &memoryBarrier, 0, 0, 0, 0);
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }

    {
        topASInfo.dstAccelerationStructure = topAS;
        topASInfo.scratchData.deviceAddress = scratchAddress;

        VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
            1, 0, 0, 0,
        };

        const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfo = &buildRangeInfo;

        // Build the top-level acceleration structure
        m_command_buffer.Begin();

        vk::CmdBuildAccelerationStructuresKHR(m_command_buffer.handle(), 1, &topASInfo, &pBuildRangeInfo);
        VkMemoryBarrier memoryBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER, NULL,
            VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR };
        vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0, 1, &memoryBarrier, 0, 0, 0, 0);

        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();
    }

    vk::DestroyAccelerationStructureKHR(*m_device, topAS, NULL);
    vk::DestroyAccelerationStructureKHR(*m_device, bottomAS, NULL);
    vk::DestroyMicromapEXT(*m_device, micromap, NULL);
}

TEST_F(PositiveRayTracing, SerializeAccelerationStructure) {
    TEST_DESCRIPTION("Build a list of destination acceleration structures, then do an update build on that same list");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayQuery);
    RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
    RETURN_IF_SKIP(InitState());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD: base buffer device addresses must be aligned";
    }

    auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    blas.AddFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR);

    m_command_buffer.Begin();

    blas.BuildCmdBuffer(m_command_buffer.handle());

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    vkt::QueryPool serialization_query_pool(*m_device, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, 1);

    m_command_buffer.Begin();

    vk::CmdResetQueryPool(m_command_buffer.handle(), serialization_query_pool.handle(), 0, 1);
    vk::CmdWriteAccelerationStructuresPropertiesKHR(m_command_buffer.handle(), 1, &blas.GetDstAS()->handle(),
                                                    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR,
                                                    serialization_query_pool.handle(), 0);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
    uint32_t accel_struct_serialization_size = 0;
    serialization_query_pool.Results(0, 1, sizeof(uint32_t), &accel_struct_serialization_size, 0);

    // See https://vkdoc.net/man/vkCmdCopyAccelerationStructureToMemoryKHR
    const VkDeviceSize serialized_accel_struct_size =
        Align<VkDeviceSize>(2 * VK_UUID_SIZE + 3 * sizeof(uint64_t) * accel_struct_serialization_size, 256);
    vkt::Buffer serialized_accel_struct_buffer(
        *m_device, serialized_accel_struct_size,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        vkt::device_address);

    VkCopyAccelerationStructureToMemoryInfoKHR copy_accel_struct_to_memory_info = vku::InitStructHelper();
    copy_accel_struct_to_memory_info.src = blas.GetDstAS()->handle();
    copy_accel_struct_to_memory_info.dst.deviceAddress = serialized_accel_struct_buffer.Address();
    copy_accel_struct_to_memory_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;

    m_command_buffer.Begin();
    vk::CmdCopyAccelerationStructureToMemoryKHR(m_command_buffer.handle(), &copy_accel_struct_to_memory_info);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    vkt::Buffer de_serialized_accel_struct_buffer(
        *m_device, serialized_accel_struct_size,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        vkt::device_address);

    auto deserialized_blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
    deserialized_blas.AddFlags(
        VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR /*| VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR*/);
    deserialized_blas.GetDstAS()->SetSize(Align<VkDeviceSize>(serialized_accel_struct_size, 256));
    deserialized_blas.GetDstAS()->Build();

    VkCopyMemoryToAccelerationStructureInfoKHR copy_memory_to_accel_struct_info = vku::InitStructHelper();
    copy_memory_to_accel_struct_info.src.deviceAddress = serialized_accel_struct_buffer.Address();
    copy_memory_to_accel_struct_info.dst = deserialized_blas.GetDstAS()->handle();
    copy_memory_to_accel_struct_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    m_command_buffer.Begin();

    vk::CmdCopyMemoryToAccelerationStructureKHR(m_command_buffer.handle(), &copy_memory_to_accel_struct_info);

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    deserialized_blas.SetSrcAS(deserialized_blas.GetDstAS());
    deserialized_blas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR);

    m_command_buffer.Begin();

    deserialized_blas.BuildCmdBuffer(m_command_buffer.handle());

    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();
}
