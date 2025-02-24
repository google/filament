/*
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "binding.h"
#include "descriptor_helper.h"
#include "shader_helper.h"

#include <memory>
#include <optional>

namespace vkt {
// acceleration structure
namespace as {

// Helper classes to create instances of:
// - VkAccelerationStructureGeometryKHR
// - VkAccelerationStructureCreateInfoKHR
// - VkAccelerationStructureBuildGeometryInfoKHR

// The vkt::as::blueprint namespace (bottom of file) contains functions to readily create a valid instance of those classes.
// Those instances are typically modified using the available public methods.
// When done with modifications, call the Build() method to build the internal Vulkan objects.
// Access them using relevant methods, eg: handle(), GetVkObj()...

// 3 types of geometry handled: Triangle, AABB and VkAccelerationStructureInstanceKHR (used in top level acceleration structures)
// Those objects can be managed on the device, using the ***Device*** methods,
// or on the host using the ***Host*** methods
class GeometryKHR {
  public:
    enum class Type { Triangle, AABB, Instance, _INTERNAL_UNSPECIFIED };
    struct Triangles {
        vkt::Buffer device_vertex_buffer;
        std::unique_ptr<float[]> host_vertex_buffer;
        vkt::Buffer device_index_buffer;
        std::unique_ptr<uint32_t[]> host_index_buffer;
        vkt::Buffer device_transform_buffer;
    };
    struct AABBs {
        vkt::Buffer device_buffer;
        std::unique_ptr<VkAabbPositionsKHR[]> host_buffer;
    };
    struct Instances {
        std::vector<VkAccelerationStructureInstanceKHR> vk_instances{};
        // Used to (eventually, no need for host instance) store on device instances
        vkt::Buffer buffer;
    };

    ~GeometryKHR() = default;
    GeometryKHR();
    GeometryKHR(const GeometryKHR&) = delete;
    GeometryKHR(GeometryKHR&&) = default;
    GeometryKHR& operator=(GeometryKHR&&) = default;
    GeometryKHR& operator=(const GeometryKHR&) = delete;

    // Common methods for all types
    GeometryKHR& SetFlags(VkGeometryFlagsKHR flags);
    GeometryKHR& SetType(Type type);
    GeometryKHR& SetPrimitiveCount(uint32_t primitiveCount);
    GeometryKHR& SetStride(VkDeviceSize stride);
    // Triangle
    GeometryKHR& SetTrianglesDeviceVertexBuffer(vkt::Buffer&& vertex_buffer, uint32_t max_vertex,
                                                VkFormat vertex_format = VK_FORMAT_R32G32B32_SFLOAT,
                                                VkDeviceSize stride = 3 * sizeof(float));
    GeometryKHR& SetTrianglesHostVertexBuffer(std::unique_ptr<float[]>&& vertex_buffer, uint32_t max_vertex,
                                              VkDeviceSize stride = 3 * sizeof(float));
    GeometryKHR& SetTrianglesDeviceIndexBuffer(vkt::Buffer&& index_buffer, VkIndexType index_type = VK_INDEX_TYPE_UINT32);
    GeometryKHR& SetTrianglesHostIndexBuffer(std::unique_ptr<uint32_t[]> index_buffer);
    GeometryKHR& SetTrianglesIndexType(VkIndexType index_type);
    GeometryKHR& SetTrianglesVertexFormat(VkFormat vertex_format);
    GeometryKHR& SetTrianglesMaxVertex(uint32_t max_vertex);
    GeometryKHR& SetTrianglesTransformBuffer(vkt::Buffer&& transform_buffer);
    GeometryKHR& SetTrianglesTransformatData(VkDeviceAddress address);
    GeometryKHR& SetTrianglesVertexBufferDeviceAddress(VkDeviceAddress address);
    GeometryKHR& SetTrianglesIndexBufferDeviceAddress(VkDeviceAddress address);
    // AABB
    GeometryKHR& SetAABBsDeviceBuffer(vkt::Buffer&& buffer, VkDeviceSize stride = sizeof(VkAabbPositionsKHR));
    GeometryKHR& SetAABBsHostBuffer(std::unique_ptr<VkAabbPositionsKHR[]> buffer, VkDeviceSize stride = sizeof(VkAabbPositionsKHR));
    GeometryKHR& SetAABBsStride(VkDeviceSize stride);
    GeometryKHR& SetAABBsDeviceAddress(VkDeviceAddress address);

    // Instance
    GeometryKHR& AddInstanceDeviceAccelStructRef(const vkt::Device& device, VkAccelerationStructureKHR blas,
                                                 const VkAccelerationStructureInstanceKHR& instance);
    GeometryKHR& AddInstanceHostAccelStructRef(VkAccelerationStructureKHR blas);
    GeometryKHR& SetInstancesDeviceAddress(VkDeviceAddress address);
    GeometryKHR& SetInstanceHostAccelStructRef(VkAccelerationStructureKHR blas, uint32_t instance_i);
    GeometryKHR& SetInstanceHostAddress(void* address);
    GeometryKHR& SetInstanceShaderBindingTableRecordOffset(uint32_t instance_i, uint32_t instance_sbt_record_offset);

    GeometryKHR& Build();

    const auto& GetVkObj() const { return vk_obj_; }
    VkAccelerationStructureBuildRangeInfoKHR GetFullBuildRange() const;
    const auto& GetTriangles() const { return triangles_; }
    const auto& GetAABBs() const { return aabbs_; }
    auto& GetInstance() { return instances_; }
    VkGeometryFlagsKHR GetFlags() { return vk_obj_.flags; };

  private:
    VkAccelerationStructureGeometryKHR vk_obj_;
    Type type_ = Type::_INTERNAL_UNSPECIFIED;
    uint32_t primitive_count_ = 0;
    Triangles triangles_;
    AABBs aabbs_;
    Instances instances_;
};

class AccelerationStructureKHR : public vkt::internal::NonDispHandle<VkAccelerationStructureKHR> {
  public:
    ~AccelerationStructureKHR() { Destroy(); }
    AccelerationStructureKHR(const vkt::Device* device);
    AccelerationStructureKHR(AccelerationStructureKHR&& rhs) = default;
    AccelerationStructureKHR& operator=(AccelerationStructureKHR&&) = default;
    AccelerationStructureKHR& operator=(const AccelerationStructureKHR&) = delete;

    AccelerationStructureKHR& SetSize(VkDeviceSize size);
    AccelerationStructureKHR& SetOffset(VkDeviceSize offset);
    AccelerationStructureKHR& SetType(VkAccelerationStructureTypeKHR type);
    AccelerationStructureKHR& SetFlags(VkAccelerationStructureCreateFlagsKHR flags);
    AccelerationStructureKHR& SetDeviceBuffer(vkt::Buffer&& buffer);
    AccelerationStructureKHR& SetDeviceBufferMemoryAllocateFlags(VkMemoryAllocateFlags memory_allocate_flags);
    AccelerationStructureKHR& SetDeviceBufferMemoryPropertyFlags(VkMemoryPropertyFlags memory_property_flags);
    AccelerationStructureKHR& SetDeviceBufferInitNoMem(bool buffer_init_no_mem);
    // Set it to 0 to skip buffer initialization at Build() step
    AccelerationStructureKHR& SetBufferUsageFlags(VkBufferUsageFlags usage_flags);

    VkDeviceAddress GetBufferDeviceAddress() const;
    VkDeviceAddress GetAccelerationStructureDeviceAddress() const;

    // Null check is done in BuildGeometryInfoKHR::Build(). Object is build iff it is not null.
    void SetNull(bool is_null) { is_null_ = is_null; }
    bool IsNull() const { return is_null_; }
    void Build();
    bool IsBuilt() const { return initialized(); }
    void Destroy();

    auto& GetBuffer() { return device_buffer_; }

  private:
    const vkt::Device* device_;
    bool is_null_ = false;
    VkAccelerationStructureCreateInfoKHR vk_info_;
    vkt::Buffer device_buffer_;
    VkMemoryAllocateFlags buffer_memory_allocate_flags_{};
    VkMemoryPropertyFlags buffer_memory_property_flags_{};
    VkBufferUsageFlags buffer_usage_flags_{};
    bool buffer_init_no_mem_ = false;
};

class BuildGeometryInfoKHR {
  public:
    ~BuildGeometryInfoKHR() = default;
    BuildGeometryInfoKHR(const vkt::Device* device);
    BuildGeometryInfoKHR(BuildGeometryInfoKHR&&) = default;
    BuildGeometryInfoKHR& operator=(BuildGeometryInfoKHR&& rhs) = default;
    BuildGeometryInfoKHR& operator=(const BuildGeometryInfoKHR&) = delete;

    BuildGeometryInfoKHR& SetType(VkAccelerationStructureTypeKHR type);
    BuildGeometryInfoKHR& SetBuildType(VkAccelerationStructureBuildTypeKHR build_type);
    BuildGeometryInfoKHR& SetMode(VkBuildAccelerationStructureModeKHR mode);
    BuildGeometryInfoKHR& SetFlags(VkBuildAccelerationStructureFlagsKHR flags);
    BuildGeometryInfoKHR& AddFlags(VkBuildAccelerationStructureFlagsKHR flags);
    BuildGeometryInfoKHR& SetGeometries(std::vector<GeometryKHR>&& geometries);
    BuildGeometryInfoKHR& SetBuildRanges(std::vector<VkAccelerationStructureBuildRangeInfoKHR> build_range_infos);
    // Using the same pointers for src and dst is supported
    BuildGeometryInfoKHR& SetSrcAS(std::shared_ptr<AccelerationStructureKHR> src_as);
    BuildGeometryInfoKHR& SetDstAS(std::shared_ptr<AccelerationStructureKHR> dst_as);
    BuildGeometryInfoKHR& SetScratchBuffer(std::shared_ptr<vkt::Buffer> scratch_buffer);
    BuildGeometryInfoKHR& SetHostScratchBuffer(std::shared_ptr<std::vector<uint8_t>> host_scratch);
    BuildGeometryInfoKHR& SetDeviceScratchOffset(VkDeviceAddress offset);
    BuildGeometryInfoKHR& SetDeviceScratchAdditionalFlags(VkBufferUsageFlags additional_flags);
    BuildGeometryInfoKHR& SetEnableScratchBuild(bool build_scratch);
    // Should be 0 or 1
    BuildGeometryInfoKHR& SetInfoCount(uint32_t info_count);
    BuildGeometryInfoKHR& SetNullInfos(bool use_null_infos);
    BuildGeometryInfoKHR& SetNullGeometries(bool use_null_geometries);
    BuildGeometryInfoKHR& SetNullBuildRangeInfos(bool use_null_build_range_infos);
    BuildGeometryInfoKHR& SetDeferredOp(VkDeferredOperationKHR deferred_op);
    BuildGeometryInfoKHR& SetUpdateDstAccelStructSizeBeforeBuild(bool update_before_build);
    BuildGeometryInfoKHR& SetIndirectStride(uint32_t indirect_stride);
    BuildGeometryInfoKHR& SetIndirectDeviceAddress(std::optional<VkDeviceAddress> indirect_buffer_address);

    // Those functions call Build() on internal resources (geometries, src and dst acceleration structures, scratch buffer),
    // then will build/update an acceleration structure.
    void BuildCmdBuffer(VkCommandBuffer cmd_buffer, bool use_ppGeometries = true);
    void BuildCmdBufferIndirect(VkCommandBuffer cmd_buffer);
    void BuildHost();

    void UpdateDstAccelStructSize();
    void SetupBuild(bool is_on_device_build, bool use_ppGeometries = true);

    // These will only setup the geometries lists and the pertaining build ranges
    void VkCmdBuildAccelerationStructuresKHR(VkCommandBuffer cmd_buffer, bool use_ppGeometries = true);
    // TODO - indirect build not fully implemented, only cared about having a valid call at time of writing
    void VkCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer cmd_buffer);
    void VkBuildAccelerationStructuresKHR();

    auto& GetInfo() { return vk_info_; }
    auto& GetGeometries() { return geometries_; }
    auto& GetSrcAS() { return src_as_; }
    auto& GetDstAS() { return dst_as_; }
    const auto& GetScratchBuffer() const { return device_scratch_; }
    VkAccelerationStructureBuildSizesInfoKHR GetSizeInfo(bool use_ppGeometries = true);
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> GetBuildRangeInfosFromGeometries();

  private:
    friend void BuildAccelerationStructuresKHR(VkCommandBuffer cmd_buffer, std::vector<BuildGeometryInfoKHR>& infos);
    friend void BuildHostAccelerationStructuresKHR(VkDevice device, std::vector<BuildGeometryInfoKHR>& infos);

    const vkt::Device* device_;
    uint32_t vk_info_count_ = 1;
    bool use_null_infos_ = false;
    bool use_null_geometries_ = false;
    bool use_null_build_range_infos_ = false;
    bool update_dst_as_size_before_build_ = false;
    VkAccelerationStructureBuildGeometryInfoKHR vk_info_;
    VkAccelerationStructureBuildTypeKHR build_type_;
    std::vector<GeometryKHR> geometries_;
    std::shared_ptr<AccelerationStructureKHR> src_as_, dst_as_;
    bool build_scratch_ = true;
    VkDeviceAddress device_scratch_offset_ = 0;
    VkBufferUsageFlags device_scratch_additional_flags_ = 0;
    std::shared_ptr<vkt::Buffer> device_scratch_;
    std::shared_ptr<std::vector<uint8_t>> host_scratch_;
    std::unique_ptr<vkt::Buffer> indirect_buffer_;
    std::optional<VkDeviceAddress> indirect_buffer_address_{};
    uint32_t indirect_stride_ = sizeof(VkAccelerationStructureBuildRangeInfoKHR);
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> build_range_infos_;
    VkDeferredOperationKHR deferred_op_ = VK_NULL_HANDLE;
};

// Helper functions
void BuildAccelerationStructuresKHR(VkCommandBuffer cmd_buffer, std::vector<BuildGeometryInfoKHR>& infos);
void BuildHostAccelerationStructuresKHR(VkDevice device, std::vector<BuildGeometryInfoKHR>& infos);

// Helper functions providing simple, valid objects.
// Calling Build() on them without further modifications results in a usable and valid Vulkan object.
// Typical usage probably is:
// {
//    vkt::as::BuildGeometryInfoKHR as_build_info = BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device);
//
//    // for instance:
//    as_build_info.GetDstAS().SetBufferMemoryPropertyFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
//    as_build_info.SetFlags(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
//
//    m_command_buffer.Begin();
//    as_build_info.BuildCmdBuffer(*m_device, m_command_buffer.handle());
//    m_command_buffer.End();
// }
namespace blueprint {
GeometryKHR GeometrySimpleOnDeviceTriangleInfo(const vkt::Device& device, size_t triangles_count = 1);
GeometryKHR GeometrySimpleOnHostTriangleInfo();

// Cube centered at position (0,0,0), 2.0f wide
GeometryKHR GeometryCubeOnDeviceInfo(const vkt::Device& device);

GeometryKHR GeometrySimpleOnDeviceAABBInfo(const vkt::Device& device);
GeometryKHR GeometrySimpleOnHostAABBInfo();

GeometryKHR GeometrySimpleDeviceInstance(const vkt::Device& device, VkAccelerationStructureKHR device_blas);
GeometryKHR GeometrySimpleHostInstance(VkAccelerationStructureKHR host_instance);

std::shared_ptr<AccelerationStructureKHR> AccelStructNull(const vkt::Device& device);
std::shared_ptr<AccelerationStructureKHR> AccelStructSimpleOnDeviceBottomLevel(const vkt::Device& device, VkDeviceSize size);
std::shared_ptr<AccelerationStructureKHR> AccelStructSimpleOnHostBottomLevel(const vkt::Device& device, VkDeviceSize size);
std::shared_ptr<AccelerationStructureKHR> AccelStructSimpleOnDeviceTopLevel(const vkt::Device& device, VkDeviceSize size);

BuildGeometryInfoKHR BuildGeometryInfoSimpleOnDeviceBottomLevel(const vkt::Device& device,
                                                                GeometryKHR::Type geometry_type = GeometryKHR::Type::Triangle);
BuildGeometryInfoKHR BuildGeometryInfoOnDeviceBottomLevel(const vkt::Device& device, GeometryKHR&& geometry);

BuildGeometryInfoKHR BuildGeometryInfoSimpleOnHostBottomLevel(const vkt::Device& device,
                                                              GeometryKHR::Type geometry_type = GeometryKHR::Type::Triangle);

// Create an on device TLAS pointing to one BLAS
// on_device_bottom_level_geometry must have been built previously, and on the device
BuildGeometryInfoKHR BuildGeometryInfoSimpleOnDeviceTopLevel(const vkt::Device& device,
                                                             std::shared_ptr<BuildGeometryInfoKHR> on_device_blas);
// Create an on host TLAS pointing to one BLAS
// on_host_bottom_level_geometry must have been built previously, and on the host
BuildGeometryInfoKHR BuildGeometryInfoSimpleOnHostTopLevel(const vkt::Device& device,
                                                           std::shared_ptr<BuildGeometryInfoKHR> on_host_blas);

// Create and build a top level acceleration structure
BuildGeometryInfoKHR BuildOnDeviceTopLevel(const vkt::Device& device, vkt::Queue& queue, vkt::CommandBuffer& cmd_buffer);
}  // namespace blueprint
}  // namespace as

namespace rt {

struct TraceRaysSbt {
    VkStridedDeviceAddressRegionKHR ray_gen_sbt{};
    VkStridedDeviceAddressRegionKHR miss_sbt{};
    VkStridedDeviceAddressRegionKHR hit_sbt{};
    VkStridedDeviceAddressRegionKHR callable_sbt{};
};

class Pipeline {
  public:
    Pipeline(VkLayerTest& test, vkt::Device* device);
    ~Pipeline();

    // Build settings
    // --------------
    void AddCreateInfoFlags(VkPipelineCreateFlags flags);
    void InitLibraryInfo();

    void AddBinding(VkDescriptorType descriptor_type, uint32_t binding, uint32_t descriptor_count = 1);
    void CreateDescriptorSet();

    void SetPushConstantRangeSize(uint32_t byte_size);
    void SetGlslRayGenShader(const char* glsl);
    void AddSpirvRayGenShader(const char* spirv, const char* entry_point);
    void AddGlslMissShader(const char* glsl);
    void AddSpirvMissShader(const char* spirv, const char* entry_point);
    void AddGlslClosestHitShader(const char* glsl);
    void AddSpirvClosestHitShader(const char* spirv, const char* entry_point);
    void AddLibrary(const Pipeline& library);
    void AddDynamicState(VkDynamicState dynamic_state);

    // Build
    // -----
    void Build();
    void BuildPipeline();
    void BuildSbt();
    void DeferBuild();

    // Get
    // ---
    const auto& Handle() { return rt_pipeline_; }
    vkt::PipelineLayout& GetPipelineLayout() { return pipeline_layout_; }
    OneOffDescriptorSet& GetDescriptorSet() {
        assert(desc_set_);
        return *desc_set_;
    }
    TraceRaysSbt GetTraceRaysSbt(uint32_t ray_gen_shader_i = 0);
    vkt::Buffer GetTraceRaysSbtIndirectBuffer(uint32_t ray_gen_shader_i, uint32_t width, uint32_t height, uint32_t depth);
    uint32_t GetShaderGroupsCount();
    std::vector<uint8_t> GetRayTracingShaderGroupHandles();
    std::vector<uint8_t> GetRayTracingCaptureReplayShaderGroupHandles();

  private:
    VkLayerTest& test_;
    vkt::Device* device_;
    VkRayTracingPipelineCreateInfoKHR vk_info_{};
    uint32_t push_constant_range_size_ = 0;
    std::vector<VkDescriptorSetLayoutBinding> bindings_{};
    std::unique_ptr<OneOffDescriptorSet> desc_set_{};
    vkt::PipelineLayout pipeline_layout_{};
    std::vector<VkDynamicState> dynamic_states{};
    std::vector<std::unique_ptr<VkShaderObj>> ray_gen_shaders_{};
    std::vector<std::unique_ptr<VkShaderObj>> miss_shaders_{};
    std::vector<std::unique_ptr<VkShaderObj>> closest_hit_shaders_ {};
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_group_cis_{};
    vkt::Pipeline rt_pipeline_{};
    VkDeferredOperationKHR deferred_op_ = VK_NULL_HANDLE;
    vkt::Buffer sbt_buffer_{};
    VkRayTracingPipelineInterfaceCreateInfoKHR rt_pipeline_interface_info_{};
    VkPipelineLibraryCreateInfoKHR pipeline_lib_info_{};
    std::vector<VkPipeline> libraries_{};
};
}  // namespace rt

}  // namespace vkt
