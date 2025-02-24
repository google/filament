/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Copyright (c) 2020-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/buffer_helper.h"
#include "../framework/gpu_av_helper.h"
#include "../framework/ray_tracing_objects.h"

class NegativeDebugPrintfRayTracing : public DebugPrintfTests {
  public:
    void InitFrameworkWithPrintfBufferSize(uint32_t printf_buffer_size) {
        SetTargetApiVersion(VK_API_VERSION_1_1);
        AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

        VkBool32 printf_value = VK_TRUE;
        VkLayerSettingEXT printf_enable_setting = {OBJECT_LAYER_NAME, "printf_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                                   &printf_value};

        VkLayerSettingEXT printf_buffer_size_setting = {OBJECT_LAYER_NAME, "printf_buffer_size", VK_LAYER_SETTING_TYPE_UINT32_EXT,
                                                        1, &printf_buffer_size};

        std::array<VkLayerSettingEXT, 2> layer_settings = {printf_enable_setting, printf_buffer_size_setting};
        VkLayerSettingsCreateInfoEXT layer_settings_create_info = vku::InitStructHelper();
        layer_settings_create_info.settingCount = static_cast<uint32_t>(layer_settings.size());
        layer_settings_create_info.pSettings = layer_settings.data();
        RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));
        if (!CanEnableGpuAV(*this)) {
            GTEST_SKIP() << "Requirements for GPU-AV/Printf are not met";
        }
        if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
            GTEST_SKIP() << "Currently disabled for Portability";
        }
    }

    // Build Top Level Acceleration Structure:
    // 2 instances of the cube, at different positions
    // clang-format off
    /*
        cube instance 2, translation (x = 0, y = 0, z = 50)
        +----+
        |  2 |
        +----+

           Z
           ^
           |            +----+
           +---> X      |  1 | cube instance 1, translation (x = 50, y = 0, z = 0)
                        +----+
     */
    // clang-format on
    vkt::as::BuildGeometryInfoKHR GetCubesTLAS(std::shared_ptr<vkt::as::BuildGeometryInfoKHR>& out_cube_blas) {
        vkt::as::GeometryKHR cube(vkt::as::blueprint::GeometryCubeOnDeviceInfo(*m_device));
        out_cube_blas = std::make_shared<vkt::as::BuildGeometryInfoKHR>(
            vkt::as::blueprint::BuildGeometryInfoOnDeviceBottomLevel(*m_device, std::move(cube)));

        // Build Bottom Level Acceleration Structure
        m_command_buffer.Begin();
        out_cube_blas->BuildCmdBuffer(m_command_buffer.handle());
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();

        vkt::as::BuildGeometryInfoKHR tlas(m_device);

        tlas.SetType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
        tlas.SetBuildType(VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR);
        tlas.SetMode(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);

        std::vector<vkt::as::GeometryKHR> cube_instances(1);
        cube_instances[0].SetType(vkt::as::GeometryKHR::Type::Instance);

        VkAccelerationStructureInstanceKHR cube_instance_1{};
        cube_instance_1.transform.matrix[0][0] = 1.0f;
        cube_instance_1.transform.matrix[1][1] = 1.0f;
        cube_instance_1.transform.matrix[2][2] = 1.0f;
        cube_instance_1.transform.matrix[0][3] = 50.0f;
        cube_instance_1.transform.matrix[1][3] = 0.0f;
        cube_instance_1.transform.matrix[2][3] = 0.0f;
        cube_instance_1.mask = 0xff;
        cube_instance_1.instanceCustomIndex = 0;
        // Cube instance 1 will be associated to closest hit shader 1
        cube_instance_1.instanceShaderBindingTableRecordOffset = 0;
        cube_instances[0].AddInstanceDeviceAccelStructRef(*m_device, out_cube_blas->GetDstAS()->handle(), cube_instance_1);

        VkAccelerationStructureInstanceKHR cube_instance_2{};
        cube_instance_2.transform.matrix[0][0] = 1.0f;
        cube_instance_2.transform.matrix[1][1] = 1.0f;
        cube_instance_2.transform.matrix[2][2] = 1.0f;
        cube_instance_2.transform.matrix[0][3] = 0.0f;
        cube_instance_2.transform.matrix[1][3] = 0.0f;
        cube_instance_2.transform.matrix[2][3] = 50.0f;
        cube_instance_2.mask = 0xff;
        cube_instance_2.instanceCustomIndex = 0;
        // Cube instance 2 will be associated to closest hit shader 2
        cube_instance_2.instanceShaderBindingTableRecordOffset = 1;
        cube_instances[0].AddInstanceDeviceAccelStructRef(*m_device, out_cube_blas->GetDstAS()->handle(), cube_instance_2);

        tlas.SetGeometries(std::move(cube_instances));
        tlas.SetBuildRanges(tlas.GetBuildRangeInfosFromGeometries());

        // Set source and destination acceleration structures info. Does not create handles, it is done in Build()
        tlas.SetSrcAS(vkt::as::blueprint::AccelStructNull(*m_device));
        auto dstAsSize = tlas.GetSizeInfo().accelerationStructureSize;
        auto dst_as = vkt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(*m_device, dstAsSize);
        dst_as->SetType(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
        tlas.SetDstAS(std::move(dst_as));
        tlas.SetUpdateDstAccelStructSizeBeforeBuild(true);

        tlas.SetInfoCount(1);
        tlas.SetNullInfos(false);
        tlas.SetNullBuildRangeInfos(false);

        m_command_buffer.Begin();
        tlas.BuildCmdBuffer(m_command_buffer.handle());
        m_command_buffer.End();

        m_default_queue->Submit(m_command_buffer);
        m_device->Wait();

        return tlas;
    }
};

const char* GetSlangShader1() {
    // slang shader. Compiled with:
    // slangc.exe -capability GL_EXT_debug_printf .\ray_tracing.slang -target spirv-asm -profile glsl_460 -O0 -o ray_tracing.spv
    // /!\ /!\ /!\ you need to manually edit the OpEntryPoint in the resulting spir-v to match the slang shader
    /*
[[vk::binding(0, 0)]] uniform RaytracingAccelerationStructure tlas;
[[vk::binding(1, 0)]] RWStructuredBuffer<uint32_t> debug_buffer;

struct RayPayload {
    float3 hit;
};

[shader("raygeneration")]
void rayGenShader()
{
    printf("In Raygen 1");
    InterlockedAdd(debug_buffer[0], 1);
    RayPayload ray_payload = { float3(0) };
    RayDesc ray;
    ray.TMin = 0.01;
    ray.TMax = 1000.0;

    // Will miss
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,-1);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

    // Will hit cube 1
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(1,0,0);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

    // Will miss
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(-1,0,0);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);
}

[shader("raygeneration")]
void rayGenShader2()
{
    printf("In Raygen 2");
    InterlockedAdd(debug_buffer[1], 1);
    RayPayload ray_payload = { float3(0) };
    RayDesc ray;
    ray.TMin = 0.01;
    ray.TMax = 1000.0;

    // Will hit cube 1
    ray.Origin = float3(-50,0,0);
    ray.Direction = float3(1,0,0);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

    // Will hit cube 2
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,1);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);
}

[shader("miss")]
void missShader(inout RayPayload payload)
{
    printf("In Miss 1");
    InterlockedAdd(debug_buffer[2], 1);
    payload.hit = float3(0.1, 0.2, 0.3);
}

[shader("miss")]
void missShader2(inout RayPayload payload)
{
    printf("In Miss 2");
    InterlockedAdd(debug_buffer[3], 1);
    payload.hit = float3(42, 42, 42);
}

[shader("closesthit")]
void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    printf("In Closest Hit 1");
    InterlockedAdd(debug_buffer[4], 1);
    const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
attr.barycentrics.y); payload.hit = barycentric_coords;

    RayPayload ray_payload = { float3(0) };
    RayDesc ray;

    ray.TMin = 0.01;
    ray.TMax = 1000.0;
    const uint32_t miss_shader_i = 1;

    // Supposed to hit cube 2, and invoke closestHitShader2
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,1);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, miss_shader_i, ray, ray_payload);

    // Supposed to miss, and call missShader2
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,1,0);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, miss_shader_i, ray, ray_payload);
}

[shader("closesthit")]
void closestHitShader2(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    printf("In Closest Hit 2");
    InterlockedAdd(debug_buffer[5], 1);
    const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
attr.barycentrics.y); payload.hit = 999 * barycentric_coords;
}
    */
    const char* ray_tracing_pipeline_spv = R"asm(
; SPIR-V
; Version: 1.5
; Generator: Khronos; 40
; Bound: 236
; Schema: 0
OpCapability RayTracingKHR
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpExtension "SPV_KHR_ray_tracing"
%11 = OpExtInstImport "NonSemantic.DebugPrintf"
OpMemoryModel Logical GLSL450
OpEntryPoint RayGenerationKHR %rayGenShader "rayGenShader" %p %debug_buffer %tlas
OpEntryPoint RayGenerationKHR %rayGenShader2 "rayGenShader2" %p %debug_buffer %tlas
OpEntryPoint MissKHR %missShader "missShader" %debug_buffer %136
OpEntryPoint MissKHR %missShader2 "missShader2" %debug_buffer %151
OpEntryPoint ClosestHitKHR %closestHitShader "closestHitShader" %p %debug_buffer %tlas %178 %168
OpEntryPoint ClosestHitKHR %closestHitShader2 "closestHitShader2" %debug_buffer %230 %221

; Debug Information
OpSource Slang 1
%12 = OpString "In Raygen 1"
%91 = OpString "In Raygen 2"
%132 = OpString "In Miss 1"
%148 = OpString "In Miss 2"
%161 = OpString "In Closest Hit 1"
%217 = OpString "In Closest Hit 2"
OpName %RayDesc "RayDesc"                           ; id %5
OpMemberName %RayDesc 0 "Origin"
OpMemberName %RayDesc 1 "TMin"
OpMemberName %RayDesc 2 "Direction"
OpMemberName %RayDesc 3 "TMax"
OpName %ray "ray"                                   ; id %9
OpName %ray "ray"                                   ; id %9
OpName %RWStructuredBuffer "RWStructuredBuffer"     ; id %18
OpName %debug_buffer "debug_buffer"                 ; id %21
OpName %RayPayload "RayPayload"                     ; id %27
OpMemberName %RayPayload 0 "hit"
OpName %ray_payload "ray_payload"                   ; id %28
OpName %p "p"                                       ; id %49
OpName %tlas "tlas"                                 ; id %58
OpName %ray_payload_0 "ray_payload"                 ; id %61
OpName %ray_payload_1 "ray_payload"                 ; id %74
OpName %rayGenShader "rayGenShader"                 ; id %2
OpName %ray_0 "ray"                                 ; id %89
OpName %ray_0 "ray"                                 ; id %89
OpName %ray_payload_2 "ray_payload"                 ; id %95
OpName %ray_payload_3 "ray_payload"                 ; id %115
OpName %rayGenShader2 "rayGenShader2"               ; id %87
OpName %missShader "missShader"                     ; id %129
OpName %missShader2 "missShader2"                   ; id %145
OpName %ray_1 "ray"                                 ; id %159
OpName %ray_1 "ray"                                 ; id %159
OpName %BuiltInTriangleIntersectionAttributes "BuiltInTriangleIntersectionAttributes"   ; id %165
OpMemberName %BuiltInTriangleIntersectionAttributes 0 "barycentrics"
OpName %barycentric_coords "barycentric_coords"     ; id %177
OpName %ray_payload_4 "ray_payload"                 ; id %182
OpName %ray_payload_5 "ray_payload"                 ; id %201
OpName %closestHitShader "closestHitShader"         ; id %157
OpName %barycentric_coords_0 "barycentric_coords"   ; id %229
OpName %closestHitShader2 "closestHitShader2"       ; id %214

; Annotations
OpMemberDecorate %RayDesc 0 Offset 0
OpMemberDecorate %RayDesc 1 Offset 12
OpMemberDecorate %RayDesc 2 Offset 16
OpMemberDecorate %RayDesc 3 Offset 28
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %RWStructuredBuffer Block
OpMemberDecorate %RWStructuredBuffer 0 Offset 0
OpDecorate %debug_buffer Binding 1
OpDecorate %debug_buffer DescriptorSet 0
OpMemberDecorate %RayPayload 0 Offset 0
OpDecorate %p Location 0
OpDecorate %tlas Binding 0
OpDecorate %tlas DescriptorSet 0
OpMemberDecorate %BuiltInTriangleIntersectionAttributes 0 Offset 0

; Types, variables and constants
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v3float = OpTypeVector %float 3
%RayDesc = OpTypeStruct %v3float %float %v3float %float
%_ptr_Function_RayDesc = OpTypePointer Function %RayDesc
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%uint = OpTypeInt 32 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint        ; ArrayStride 4
%RWStructuredBuffer = OpTypeStruct %_runtimearr_uint    ; Block
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%float_0 = OpConstant %float 0
%25 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%RayPayload = OpTypeStruct %v3float
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_0_00999999978 = OpConstant %float 0.00999999978
%int_3 = OpConstant %int 3
%float_1000 = OpConstant %float 1000
%_ptr_Function_v3float = OpTypePointer Function %v3float
%40 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%int_2 = OpConstant %int 2
%float_n1 = OpConstant %float -1
%44 = OpConstantComposite %v3float %float_0 %float_0 %float_n1
%_ptr_RayPayloadKHR_RayPayload = OpTypePointer RayPayloadKHR %RayPayload
%55 = OpTypeAccelerationStructureKHR
%_ptr_UniformConstant_55 = OpTypePointer UniformConstant %55
%uint_255 = OpConstant %uint 255
%float_1 = OpConstant %float 1
%63 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%76 = OpConstantComposite %v3float %float_n1 %float_0 %float_0
%94 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_n50 = OpConstant %float -50
%101 = OpConstantComposite %v3float %float_n50 %float_0 %float_0
%105 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%116 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%118 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%_ptr_IncomingRayPayloadKHR_RayPayload = OpTypePointer IncomingRayPayloadKHR %RayPayload
%_ptr_IncomingRayPayloadKHR_v3float = OpTypePointer IncomingRayPayloadKHR %v3float
%float_0_100000001 = OpConstant %float 0.100000001
%float_0_200000003 = OpConstant %float 0.200000003
%float_0_300000012 = OpConstant %float 0.300000012
%139 = OpConstantComposite %v3float %float_0_100000001 %float_0_200000003 %float_0_300000012
%float_42 = OpConstant %float 42
%153 = OpConstantComposite %v3float %float_42 %float_42 %float_42
%int_4 = OpConstant %int 4
%v2float = OpTypeVector %float 2
%BuiltInTriangleIntersectionAttributes = OpTypeStruct %v2float
%_ptr_HitAttributeKHR_BuiltInTriangleIntersectionAttributes = OpTypePointer HitAttributeKHR %BuiltInTriangleIntersectionAttributes
%_ptr_HitAttributeKHR_v2float = OpTypePointer HitAttributeKHR %v2float
%181 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%188 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%191 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%203 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%int_5 = OpConstant %int 5
%float_999 = OpConstant %float 999
%debug_buffer = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer     ; Binding 1, DescriptorSet 0
%p = OpVariable %_ptr_RayPayloadKHR_RayPayload RayPayloadKHR                        ; Location 0
%tlas = OpVariable %_ptr_UniformConstant_55 UniformConstant                         ; Binding 0, DescriptorSet 0
%136 = OpVariable %_ptr_IncomingRayPayloadKHR_RayPayload IncomingRayPayloadKHR
%151 = OpVariable %_ptr_IncomingRayPayloadKHR_RayPayload IncomingRayPayloadKHR
%168 = OpVariable %_ptr_HitAttributeKHR_BuiltInTriangleIntersectionAttributes HitAttributeKHR
%178 = OpVariable %_ptr_IncomingRayPayloadKHR_RayPayload IncomingRayPayloadKHR
%221 = OpVariable %_ptr_HitAttributeKHR_BuiltInTriangleIntersectionAttributes HitAttributeKHR
%230 = OpVariable %_ptr_IncomingRayPayloadKHR_RayPayload IncomingRayPayloadKHR

; Function rayGenShader
%rayGenShader = OpFunction %void None %3
%4 = OpLabel
%ray = OpVariable %_ptr_Function_RayDesc Function
%10 = OpExtInst %void %11 1 %12
%17 = OpAccessChain %_ptr_StorageBuffer_uint %debug_buffer %int_0 %int_0
%22 = OpAtomicIAdd %uint %17 %uint_1 %uint_0 %uint_1
%ray_payload = OpCompositeConstruct %RayPayload %25
%31 = OpAccessChain %_ptr_Function_float %ray %int_1
OpStore %31 %float_0_00999999978
%35 = OpAccessChain %_ptr_Function_float %ray %int_3
OpStore %35 %float_1000
%39 = OpAccessChain %_ptr_Function_v3float %ray %int_0
OpStore %39 %40
%43 = OpAccessChain %_ptr_Function_v3float %ray %int_2
OpStore %43 %44
%47 = OpLoad %RayDesc %ray
OpStore %p %ray_payload
%51 = OpCompositeExtract %v3float %47 0
%52 = OpCompositeExtract %v3float %47 2
%53 = OpCompositeExtract %float %47 1
%54 = OpCompositeExtract %float %47 3
%56 = OpLoad %55 %tlas
OpTraceRayKHR %56 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %51 %53 %52 %54 %p
%ray_payload_0 = OpLoad %RayPayload %p
OpStore %39 %40
OpStore %43 %63
%66 = OpLoad %RayDesc %ray
OpStore %p %ray_payload_0
%68 = OpCompositeExtract %v3float %66 0
%69 = OpCompositeExtract %v3float %66 2
%70 = OpCompositeExtract %float %66 1
%71 = OpCompositeExtract %float %66 3
%72 = OpLoad %55 %tlas
OpTraceRayKHR %72 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %68 %70 %69 %71 %p
%ray_payload_1 = OpLoad %RayPayload %p
OpStore %39 %40
OpStore %43 %76
%78 = OpLoad %RayDesc %ray
OpStore %p %ray_payload_1
%80 = OpCompositeExtract %v3float %78 0
%81 = OpCompositeExtract %v3float %78 2
%82 = OpCompositeExtract %float %78 1
%83 = OpCompositeExtract %float %78 3
%84 = OpLoad %55 %tlas
OpTraceRayKHR %84 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %80 %82 %81 %83 %p
OpReturn
OpFunctionEnd

; Function rayGenShader2
%rayGenShader2 = OpFunction %void None %3
%88 = OpLabel
%ray_0 = OpVariable %_ptr_Function_RayDesc Function
%90 = OpExtInst %void %11 1 %91
%92 = OpAccessChain %_ptr_StorageBuffer_uint %debug_buffer %int_0 %int_1
%93 = OpAtomicIAdd %uint %92 %uint_1 %uint_0 %uint_1
%ray_payload_2 = OpCompositeConstruct %RayPayload %94
%96 = OpAccessChain %_ptr_Function_float %ray_0 %int_1
OpStore %96 %float_0_00999999978
%98 = OpAccessChain %_ptr_Function_float %ray_0 %int_3
OpStore %98 %float_1000
%100 = OpAccessChain %_ptr_Function_v3float %ray_0 %int_0
OpStore %100 %101
%104 = OpAccessChain %_ptr_Function_v3float %ray_0 %int_2
OpStore %104 %105
%107 = OpLoad %RayDesc %ray_0
OpStore %p %ray_payload_2
%109 = OpCompositeExtract %v3float %107 0
%110 = OpCompositeExtract %v3float %107 2
%111 = OpCompositeExtract %float %107 1
%112 = OpCompositeExtract %float %107 3
%113 = OpLoad %55 %tlas
OpTraceRayKHR %113 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %109 %111 %110 %112 %p
%ray_payload_3 = OpLoad %RayPayload %p
OpStore %100 %116
OpStore %104 %118
%120 = OpLoad %RayDesc %ray_0
OpStore %p %ray_payload_3
%122 = OpCompositeExtract %v3float %120 0
%123 = OpCompositeExtract %v3float %120 2
%124 = OpCompositeExtract %float %120 1
%125 = OpCompositeExtract %float %120 3
%126 = OpLoad %55 %tlas
OpTraceRayKHR %126 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %122 %124 %123 %125 %p
OpReturn
OpFunctionEnd

; Function missShader
%missShader = OpFunction %void None %3
%130 = OpLabel
%131 = OpExtInst %void %11 1 %132
%133 = OpAccessChain %_ptr_StorageBuffer_uint %debug_buffer %int_0 %int_2
%134 = OpAtomicIAdd %uint %133 %uint_1 %uint_0 %uint_1
%138 = OpAccessChain %_ptr_IncomingRayPayloadKHR_v3float %136 %int_0
OpStore %138 %139
OpReturn
OpFunctionEnd

; Function missShader2
%missShader2 = OpFunction %void None %3
%146 = OpLabel
%147 = OpExtInst %void %11 1 %148
%149 = OpAccessChain %_ptr_StorageBuffer_uint %debug_buffer %int_0 %int_3
%150 = OpAtomicIAdd %uint %149 %uint_1 %uint_0 %uint_1
%152 = OpAccessChain %_ptr_IncomingRayPayloadKHR_v3float %151 %int_0
OpStore %152 %153
OpReturn
OpFunctionEnd

; Function closestHitShader
%closestHitShader = OpFunction %void None %3
%158 = OpLabel
%ray_1 = OpVariable %_ptr_Function_RayDesc Function
%160 = OpExtInst %void %11 1 %161
%163 = OpAccessChain %_ptr_StorageBuffer_uint %debug_buffer %int_0 %int_4
%164 = OpAtomicIAdd %uint %163 %uint_1 %uint_0 %uint_1
%170 = OpAccessChain %_ptr_HitAttributeKHR_v2float %168 %int_0
%171 = OpLoad %v2float %170
%172 = OpCompositeExtract %float %171 0
%173 = OpFSub %float %float_1 %172
%174 = OpLoad %v2float %170
%175 = OpCompositeExtract %float %174 1
%176 = OpFSub %float %173 %175
%barycentric_coords = OpCompositeConstruct %v3float %176 %172 %175
%179 = OpAccessChain %_ptr_IncomingRayPayloadKHR_v3float %178 %int_0
OpStore %179 %barycentric_coords
%ray_payload_4 = OpCompositeConstruct %RayPayload %181
%183 = OpAccessChain %_ptr_Function_float %ray_1 %int_1
OpStore %183 %float_0_00999999978
%185 = OpAccessChain %_ptr_Function_float %ray_1 %int_3
OpStore %185 %float_1000
%187 = OpAccessChain %_ptr_Function_v3float %ray_1 %int_0
OpStore %187 %188
%190 = OpAccessChain %_ptr_Function_v3float %ray_1 %int_2
OpStore %190 %191
%193 = OpLoad %RayDesc %ray_1
OpStore %p %ray_payload_4
%195 = OpCompositeExtract %v3float %193 0
%196 = OpCompositeExtract %v3float %193 2
%197 = OpCompositeExtract %float %193 1
%198 = OpCompositeExtract %float %193 3
%199 = OpLoad %55 %tlas
OpTraceRayKHR %199 %uint_0 %uint_255 %uint_0 %uint_0 %uint_1 %195 %197 %196 %198 %p
%ray_payload_5 = OpLoad %RayPayload %p
OpStore %187 %188
OpStore %190 %203
%205 = OpLoad %RayDesc %ray_1
OpStore %p %ray_payload_5
%207 = OpCompositeExtract %v3float %205 0
%208 = OpCompositeExtract %v3float %205 2
%209 = OpCompositeExtract %float %205 1
%210 = OpCompositeExtract %float %205 3
%211 = OpLoad %55 %tlas
OpTraceRayKHR %211 %uint_0 %uint_255 %uint_0 %uint_0 %uint_1 %207 %209 %208 %210 %p
OpReturn
OpFunctionEnd

; Function closestHitShader2
%closestHitShader2 = OpFunction %void None %3
%215 = OpLabel
%216 = OpExtInst %void %11 1 %217
%219 = OpAccessChain %_ptr_StorageBuffer_uint %debug_buffer %int_0 %int_5
%220 = OpAtomicIAdd %uint %219 %uint_1 %uint_0 %uint_1
%222 = OpAccessChain %_ptr_HitAttributeKHR_v2float %221 %int_0
%223 = OpLoad %v2float %222
%224 = OpCompositeExtract %float %223 0
%225 = OpFSub %float %float_1 %224
%226 = OpLoad %v2float %222
%227 = OpCompositeExtract %float %226 1
%228 = OpFSub %float %225 %227
%barycentric_coords_0 = OpCompositeConstruct %v3float %228 %224 %227
%231 = OpAccessChain %_ptr_IncomingRayPayloadKHR_v3float %230 %int_0
%232 = OpVectorTimesScalar %v3float %barycentric_coords_0 %float_999
OpStore %231 %232
OpReturn
OpFunctionEnd
    )asm";

    return ray_tracing_pipeline_spv;
}

const char* GetSlangShader2() {
    // slang shader. Compiled with:
    // slangc.exe -capability GL_EXT_debug_printf .\ray_tracing.slang -target spirv-asm -profile glsl_460 -O0 -o ray_tracing.spv
    /*
[[vk::binding(0, 0)]] uniform RaytracingAccelerationStructure tlas;
[[vk::binding(1, 0)]] RWStructuredBuffer<uint32_t> debug_buffer;

struct RayPayload {
    float3 hit;
};

[shader("raygeneration")]
void rayGenShader()
{
    printf("In Raygen");
    InterlockedAdd(debug_buffer[0], 1);
    RayPayload ray_payload = { float3(0) };
    RayDesc ray;
    ray.TMin = 0.01;
    ray.TMax = 1000.0;

    // Will hit
    ray.Origin = float3(0,0,-50);
    ray.Direction = float3(0,0,1);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

    // Will miss
    ray.Origin = float3(0,0,-50);
    ray.Direction = float3(0,0,-1);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

    // Will miss
    ray.Origin = float3(0,0,50);
    ray.Direction = float3(0,0,1);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

    // Will miss
    ray.Origin = float3(0,0,50);
    ray.Direction = float3(0,0,-1);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

    // Will miss
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,1);
    TraceRay(tlas, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, ray_payload);

}

[shader("miss")]
void missShader(inout RayPayload payload)
{
    printf("In Miss");
    InterlockedAdd(debug_buffer[1], 1);
    payload.hit = float3(0.1, 0.2, 0.3);
}

[shader("closesthit")]
void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    printf("In Closest Hit");
    InterlockedAdd(debug_buffer[2], 1);
    const float3 barycentric_coords = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
attr.barycentrics.y); payload.hit = barycentric_coords;
}
    */
    const char* ray_tracing_pipeline_spv = R"asm(
; SPIR-V
; Version: 1.5
; Generator: Khronos; 40
; Bound: 151
; Schema: 0
OpCapability RayTracingKHR
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpExtension "SPV_KHR_ray_tracing"
%11 = OpExtInstImport "NonSemantic.DebugPrintf"
OpMemoryModel Logical GLSL450
OpEntryPoint RayGenerationKHR %rayGenShader "main" %p %debug_buffer %tlas
OpEntryPoint MissKHR %missShader "main" %debug_buffer %119
OpEntryPoint ClosestHitKHR %closestHitShader "main" %debug_buffer %147 %137

; Debug Information
OpSource Slang 1
%12 = OpString "In Raygen"
%115 = OpString "In Miss"
%131 = OpString "In Closest Hit"
OpName %RayDesc "RayDesc"                           ; id %5
OpMemberName %RayDesc 0 "Origin"
OpMemberName %RayDesc 1 "TMin"
OpMemberName %RayDesc 2 "Direction"
OpMemberName %RayDesc 3 "TMax"
OpName %ray "ray"                                   ; id %9
OpName %ray "ray"                                   ; id %9
OpName %RWStructuredBuffer "RWStructuredBuffer"     ; id %18
OpName %debug_buffer "debug_buffer"                 ; id %21
OpName %RayPayload "RayPayload"                     ; id %27
OpMemberName %RayPayload 0 "hit"
OpName %ray_payload "ray_payload"                   ; id %28
OpName %p "p"                                       ; id %50
OpName %tlas "tlas"                                 ; id %59
OpName %ray_payload_0 "ray_payload"                 ; id %62
OpName %ray_payload_1 "ray_payload"                 ; id %75
OpName %ray_payload_2 "ray_payload"                 ; id %88
OpName %ray_payload_3 "ray_payload"                 ; id %99
OpName %rayGenShader "rayGenShader"                 ; id %2
OpName %missShader "missShader"                     ; id %112
OpName %BuiltInTriangleIntersectionAttributes "BuiltInTriangleIntersectionAttributes"   ; id %134
OpMemberName %BuiltInTriangleIntersectionAttributes 0 "barycentrics"
OpName %barycentric_coords "barycentric_coords"     ; id %146
OpName %closestHitShader "closestHitShader"         ; id %128

; Annotations
OpMemberDecorate %RayDesc 0 Offset 0
OpMemberDecorate %RayDesc 1 Offset 12
OpMemberDecorate %RayDesc 2 Offset 16
OpMemberDecorate %RayDesc 3 Offset 28
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %RWStructuredBuffer Block
OpMemberDecorate %RWStructuredBuffer 0 Offset 0
OpDecorate %debug_buffer Binding 1
OpDecorate %debug_buffer DescriptorSet 0
OpMemberDecorate %RayPayload 0 Offset 0
OpDecorate %p Location 0
OpDecorate %tlas Binding 0
OpDecorate %tlas DescriptorSet 0
OpMemberDecorate %BuiltInTriangleIntersectionAttributes 0 Offset 0

; Types, variables and constants
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v3float = OpTypeVector %float 3
%RayDesc = OpTypeStruct %v3float %float %v3float %float
%_ptr_Function_RayDesc = OpTypePointer Function %RayDesc
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%uint = OpTypeInt 32 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint        ; ArrayStride 4
%RWStructuredBuffer = OpTypeStruct %_runtimearr_uint    ; Block
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%float_0 = OpConstant %float 0
%25 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%RayPayload = OpTypeStruct %v3float
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_0_00999999978 = OpConstant %float 0.00999999978
%int_3 = OpConstant %int 3
%float_1000 = OpConstant %float 1000
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_n50 = OpConstant %float -50
%40 = OpConstantComposite %v3float %float_0 %float_0 %float_n50
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%45 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%_ptr_RayPayloadKHR_RayPayload = OpTypePointer RayPayloadKHR %RayPayload
%56 = OpTypeAccelerationStructureKHR
%_ptr_UniformConstant_56 = OpTypePointer UniformConstant %56
%uint_255 = OpConstant %uint 255
%float_n1 = OpConstant %float -1
%64 = OpConstantComposite %v3float %float_0 %float_0 %float_n1
%float_50 = OpConstant %float 50
%76 = OpConstantComposite %v3float %float_0 %float_0 %float_50
%100 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_IncomingRayPayloadKHR_RayPayload = OpTypePointer IncomingRayPayloadKHR %RayPayload
%_ptr_IncomingRayPayloadKHR_v3float = OpTypePointer IncomingRayPayloadKHR %v3float
%float_0_100000001 = OpConstant %float 0.100000001
%float_0_200000003 = OpConstant %float 0.200000003
%float_0_300000012 = OpConstant %float 0.300000012
%122 = OpConstantComposite %v3float %float_0_100000001 %float_0_200000003 %float_0_300000012
%v2float = OpTypeVector %float 2
%BuiltInTriangleIntersectionAttributes = OpTypeStruct %v2float
%_ptr_HitAttributeKHR_BuiltInTriangleIntersectionAttributes = OpTypePointer HitAttributeKHR %BuiltInTriangleIntersectionAttributes
%_ptr_HitAttributeKHR_v2float = OpTypePointer HitAttributeKHR %v2float
%debug_buffer = OpVariable %_ptr_StorageBuffer_RWStructuredBuffer StorageBuffer     ; Binding 1, DescriptorSet 0
%p = OpVariable %_ptr_RayPayloadKHR_RayPayload RayPayloadKHR                        ; Location 0
%tlas = OpVariable %_ptr_UniformConstant_56 UniformConstant                         ; Binding 0, DescriptorSet 0
%119 = OpVariable %_ptr_IncomingRayPayloadKHR_RayPayload IncomingRayPayloadKHR
%137 = OpVariable %_ptr_HitAttributeKHR_BuiltInTriangleIntersectionAttributes HitAttributeKHR
%147 = OpVariable %_ptr_IncomingRayPayloadKHR_RayPayload IncomingRayPayloadKHR

; Function rayGenShader
%rayGenShader = OpFunction %void None %3
%4 = OpLabel
%ray = OpVariable %_ptr_Function_RayDesc Function
%10 = OpExtInst %void %11 1 %12
%17 = OpAccessChain %_ptr_StorageBuffer_uint %debug_buffer %int_0 %int_0
%22 = OpAtomicIAdd %uint %17 %uint_1 %uint_0 %uint_1
%ray_payload = OpCompositeConstruct %RayPayload %25
%31 = OpAccessChain %_ptr_Function_float %ray %int_1
OpStore %31 %float_0_00999999978
%35 = OpAccessChain %_ptr_Function_float %ray %int_3
OpStore %35 %float_1000
%39 = OpAccessChain %_ptr_Function_v3float %ray %int_0
OpStore %39 %40
%44 = OpAccessChain %_ptr_Function_v3float %ray %int_2
OpStore %44 %45
%48 = OpLoad %RayDesc %ray
OpStore %p %ray_payload
%52 = OpCompositeExtract %v3float %48 0
%53 = OpCompositeExtract %v3float %48 2
%54 = OpCompositeExtract %float %48 1
%55 = OpCompositeExtract %float %48 3
%57 = OpLoad %56 %tlas
OpTraceRayKHR %57 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %52 %54 %53 %55 %p
%ray_payload_0 = OpLoad %RayPayload %p
OpStore %39 %40
OpStore %44 %64
%67 = OpLoad %RayDesc %ray
OpStore %p %ray_payload_0
%69 = OpCompositeExtract %v3float %67 0
%70 = OpCompositeExtract %v3float %67 2
%71 = OpCompositeExtract %float %67 1
%72 = OpCompositeExtract %float %67 3
%73 = OpLoad %56 %tlas
OpTraceRayKHR %73 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %69 %71 %70 %72 %p
%ray_payload_1 = OpLoad %RayPayload %p
OpStore %39 %76
OpStore %44 %45
%80 = OpLoad %RayDesc %ray
OpStore %p %ray_payload_1
%82 = OpCompositeExtract %v3float %80 0
%83 = OpCompositeExtract %v3float %80 2
%84 = OpCompositeExtract %float %80 1
%85 = OpCompositeExtract %float %80 3
%86 = OpLoad %56 %tlas
OpTraceRayKHR %86 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %82 %84 %83 %85 %p
%ray_payload_2 = OpLoad %RayPayload %p
OpStore %39 %76
OpStore %44 %64
%91 = OpLoad %RayDesc %ray
OpStore %p %ray_payload_2
%93 = OpCompositeExtract %v3float %91 0
%94 = OpCompositeExtract %v3float %91 2
%95 = OpCompositeExtract %float %91 1
%96 = OpCompositeExtract %float %91 3
%97 = OpLoad %56 %tlas
OpTraceRayKHR %97 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %93 %95 %94 %96 %p
%ray_payload_3 = OpLoad %RayPayload %p
OpStore %39 %100
OpStore %44 %45
%103 = OpLoad %RayDesc %ray
OpStore %p %ray_payload_3
%105 = OpCompositeExtract %v3float %103 0
%106 = OpCompositeExtract %v3float %103 2
%107 = OpCompositeExtract %float %103 1
%108 = OpCompositeExtract %float %103 3
%109 = OpLoad %56 %tlas
OpTraceRayKHR %109 %uint_0 %uint_255 %uint_0 %uint_0 %uint_0 %105 %107 %106 %108 %p
OpReturn
OpFunctionEnd

; Function missShader
%missShader = OpFunction %void None %3
%113 = OpLabel
%114 = OpExtInst %void %11 1 %115
%116 = OpAccessChain %_ptr_StorageBuffer_uint %debug_buffer %int_0 %int_1
%117 = OpAtomicIAdd %uint %116 %uint_1 %uint_0 %uint_1
%121 = OpAccessChain %_ptr_IncomingRayPayloadKHR_v3float %119 %int_0
OpStore %121 %122
OpReturn
OpFunctionEnd

; Function closestHitShader
%closestHitShader = OpFunction %void None %3
%129 = OpLabel
%130 = OpExtInst %void %11 1 %131
%132 = OpAccessChain %_ptr_StorageBuffer_uint %debug_buffer %int_0 %int_2
%133 = OpAtomicIAdd %uint %132 %uint_1 %uint_0 %uint_1
%139 = OpAccessChain %_ptr_HitAttributeKHR_v2float %137 %int_0
%140 = OpLoad %v2float %139
%141 = OpCompositeExtract %float %140 0
%142 = OpFSub %float %float_1 %141
%143 = OpLoad %v2float %139
%144 = OpCompositeExtract %float %143 1
%145 = OpFSub %float %142 %144
%barycentric_coords = OpCompositeConstruct %v3float %145 %141 %144
%148 = OpAccessChain %_ptr_IncomingRayPayloadKHR_v3float %147 %int_0
OpStore %148 %barycentric_coords
OpReturn
OpFunctionEnd
    )asm";

    return ray_tracing_pipeline_spv;
}

TEST_F(NegativeDebugPrintfRayTracing, Raygen) {
    TEST_DESCRIPTION("Test debug printf in raygen shader.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_debug_printf : enable
        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            debugPrintfEXT("In Raygen\n");
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, vec3(0,0,1), 0.1, vec3(0,0,1), 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    pipeline.AddGlslMissShader(kRayTracingPayloadMinimalGlsl);
    pipeline.AddGlslClosestHitShader(kRayTracingPayloadMinimalGlsl);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.CreateDescriptorSet();
    vkt::as::BuildGeometryInfoKHR tlas(vkt::as::blueprint::BuildOnDeviceTopLevel(*m_device, *m_default_queue, m_command_buffer));
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(), 0, 1,
                              &pipeline.GetDescriptorSet().set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
    vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();
    vk::CmdTraceRaysKHR(m_command_buffer, &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt, &trace_rays_sbt.hit_sbt,
                        &trace_rays_sbt.callable_sbt, 1, 1, 1);
    m_command_buffer.End();
    m_errorMonitor->SetDesiredInfo("In Raygen");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintfRayTracing, RaygenOneMissShaderOneClosestHitShader) {
    TEST_DESCRIPTION("Test debug printf in raygen, miss and closest hit shaders.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitFrameworkWithPrintfBufferSize(1024 * 1024));
    RETURN_IF_SKIP(InitState());

    // #ARNO_TODO: For clarity, here geometry should be set explicitly, as of now the ray hitting or not
    // implicitly depends on the default triangle position.
    auto blas = std::make_shared<vkt::as::BuildGeometryInfoKHR>(
        vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Triangle));

    // Build Bottom Level Acceleration Structure
    m_command_buffer.Begin();
    blas->BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, blas);
    m_command_buffer.Begin();
    tlas.BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Buffer used to count invocations for the 3 shader types
    vkt::Buffer debug_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer.handle(), debug_buffer.handle(), 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vkt::rt::Pipeline pipeline(*this, m_device);

    const char* ray_gen = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_debug_printf : enable

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) buffer DbgBuffer {
          uint debug_buffer[];
        };

        layout(location = 0) rayPayloadEXT vec3 hit;

        void main() {
            uint last = atomicAdd(debug_buffer[0], 1);
            debugPrintfEXT("In Raygen %u", last);

            vec3 ray_origin = vec3(0,0,-50);
            vec3 ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,-50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            ray_origin = vec3(0,0,50);
            ray_direction = vec3(0,0,-1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);

            // Will miss
            ray_origin = vec3(0,0,0);
            ray_direction = vec3(0,0,1);
            traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, ray_origin, 0.01, ray_direction, 1000.0, 0);
        }
    )glsl";
    pipeline.SetGlslRayGenShader(ray_gen);

    const char* miss = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_debug_printf : enable

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) buffer DbgBuffer {
          uint debug_buffer[];
        };

        layout(location = 0) rayPayloadInEXT vec3 hit;

        void main() {
            uint last = atomicAdd(debug_buffer[1], 1);
            debugPrintfEXT("In Miss %u", last);
            hit = vec3(0.1, 0.2, 0.3);
        }
    )glsl";
    pipeline.AddGlslMissShader(miss);

    const char* closest_hit = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        #extension GL_EXT_debug_printf : enable

        layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
        layout(binding = 1, set = 0) buffer DbgBuffer {
          uint debug_buffer[];
        };


        layout(location = 0) rayPayloadInEXT vec3 hit;
        hitAttributeEXT vec2 baryCoord;

        void main() {
            uint last = atomicAdd(debug_buffer[2], 1);
            debugPrintfEXT("In Closest Hit %u", last);
            const vec3 barycentricCoords = vec3(1.0f - baryCoord.x - baryCoord.y, baryCoord.x, baryCoord.y);
            hit = barycentricCoords;
        }
    )glsl";
    pipeline.AddGlslClosestHitShader(closest_hit);

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    constexpr uint32_t frames_count = 14;
    const uint32_t ray_gen_width = 1;
    const uint32_t ray_gen_height = 4;
    const uint32_t ray_gen_depth = 1;
    const uint32_t ray_gen_rays_count = ray_gen_width * ray_gen_height * ray_gen_depth;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(),
                                  0, 1, &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
        vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

        vk::CmdTraceRaysKHR(m_command_buffer.handle(), &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt,
                            &trace_rays_sbt.hit_sbt, &trace_rays_sbt.callable_sbt, ray_gen_width, ray_gen_height, ray_gen_depth);

        m_command_buffer.End();
        for (uint32_t i = 0; i < ray_gen_rays_count; ++i) {
            std::string msg = "In Raygen " + std::to_string(frame * ray_gen_rays_count + i);
            m_errorMonitor->SetDesiredInfo(msg.c_str());
        }
        for (uint32_t i = 0; i < 3 * ray_gen_rays_count; ++i) {
            std::string msg = "In Miss " + std::to_string(frame * 3 * ray_gen_rays_count + i);
            m_errorMonitor->SetDesiredInfo(msg.c_str());
        }
        for (uint32_t i = 0; i < 2 * ray_gen_rays_count; ++i) {
            std::string msg = "In Closest Hit " + std::to_string(frame * 2 * ray_gen_rays_count + i);
            m_errorMonitor->SetDesiredInfo(msg.c_str());
        }

        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();

        m_errorMonitor->VerifyFound();
    }

    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], ray_gen_rays_count * frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 3 * ray_gen_rays_count * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], 2 * ray_gen_rays_count * frames_count);
    debug_buffer.Memory().Unmap();
}

TEST_F(NegativeDebugPrintfRayTracing, OneMultiEntryPointsShader) {
    TEST_DESCRIPTION(
        "Test debug printf in a multi entry points shader. 1 ray generation shader, 1 miss shader, 1 closest hit shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(InitDebugPrintfFramework());

    RETURN_IF_SKIP(InitState());

    // #ARNO_TODO: For clarity, here geometry should be set explicitly, as of now the ray hitting or not
    // implicitly depends on the default triangle position.
    auto blas = std::make_shared<vkt::as::BuildGeometryInfoKHR>(
        vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Triangle));

    // Build Bottom Level Acceleration Structure
    m_command_buffer.Begin();
    blas->BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Build Top Level Acceleration Structure
    vkt::as::BuildGeometryInfoKHR tlas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceTopLevel(*m_device, blas);
    m_command_buffer.Begin();
    tlas.BuildCmdBuffer(m_command_buffer.handle());
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Buffer used to count invocations for the 3 shader types
    vkt::Buffer debug_buffer(*m_device, 3 * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                             kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer.handle(), debug_buffer.handle(), 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vkt::rt::Pipeline pipeline(*this, m_device);

    pipeline.AddSpirvRayGenShader(GetSlangShader2(), "main");
    pipeline.AddSpirvMissShader(GetSlangShader2(), "main");
    pipeline.AddSpirvClosestHitShader(GetSlangShader2(), "main");

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    uint32_t frames_count = 42;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(),
                                  0, 1, &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());
        vkt::rt::TraceRaysSbt trace_rays_sbt = pipeline.GetTraceRaysSbt();

        vk::CmdTraceRaysKHR(m_command_buffer.handle(), &trace_rays_sbt.ray_gen_sbt, &trace_rays_sbt.miss_sbt,
                            &trace_rays_sbt.hit_sbt, &trace_rays_sbt.callable_sbt, 1, 1, 1);

        m_command_buffer.End();
        m_errorMonitor->SetDesiredInfo("In Raygen");
        m_errorMonitor->SetDesiredInfo("In Miss", 3);
        m_errorMonitor->SetDesiredInfo("In Closest Hit", 2);
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }
    m_errorMonitor->VerifyFound();

    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 3 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], 2 * frames_count);
    debug_buffer.Memory().Unmap();
}

TEST_F(NegativeDebugPrintfRayTracing, OneMultiEntryPointsShader2CmdTraceRays) {
    TEST_DESCRIPTION(
        "Test debug printf in a multi entry points shader. 2 ray generation shaders, 2 miss shaders, 2 closest hit shaders."
        "Trace rays using vkCmdTraceRaysKHR");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    std::shared_ptr<vkt::as::BuildGeometryInfoKHR> cube_blas;
    vkt::as::BuildGeometryInfoKHR tlas = GetCubesTLAS(cube_blas);

    // Buffer used to count invocations for the 2 * 3 shaders
    vkt::Buffer debug_buffer(*m_device, 2 * 3 * sizeof(uint32_t),
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer.handle(), debug_buffer.handle(), 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vkt::rt::Pipeline pipeline(*this, m_device);

    pipeline.AddSpirvRayGenShader(GetSlangShader1(), "rayGenShader");
    pipeline.AddSpirvRayGenShader(GetSlangShader1(), "rayGenShader2");
    pipeline.AddSpirvMissShader(GetSlangShader1(), "missShader");
    pipeline.AddSpirvMissShader(GetSlangShader1(), "missShader2");
    pipeline.AddSpirvClosestHitShader(GetSlangShader1(), "closestHitShader");
    pipeline.AddSpirvClosestHitShader(GetSlangShader1(), "closestHitShader2");

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    uint32_t frames_count = 42;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(),
                                  0, 1, &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());

        // Invoke ray gen shader 1
        vkt::rt::TraceRaysSbt sbt_ray_gen_1 = pipeline.GetTraceRaysSbt(0);
        vk::CmdTraceRaysKHR(m_command_buffer.handle(), &sbt_ray_gen_1.ray_gen_sbt, &sbt_ray_gen_1.miss_sbt, &sbt_ray_gen_1.hit_sbt,
                            &sbt_ray_gen_1.callable_sbt, 1, 1, 1);

        // Invoke ray gen shader 2
        vkt::rt::TraceRaysSbt sbt_ray_gen_2 = pipeline.GetTraceRaysSbt(1);
        vk::CmdTraceRaysKHR(m_command_buffer.handle(), &sbt_ray_gen_2.ray_gen_sbt, &sbt_ray_gen_2.miss_sbt, &sbt_ray_gen_2.hit_sbt,
                            &sbt_ray_gen_2.callable_sbt, 1, 1, 1);

        m_command_buffer.End();
        m_errorMonitor->SetDesiredInfo("In Raygen 1");
        m_errorMonitor->SetDesiredInfo("In Raygen 2");
        m_errorMonitor->SetDesiredInfo("In Miss 1", 2);
        m_errorMonitor->SetDesiredInfo("In Miss 2", 2);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 1", 2);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 2", 3);
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }
    m_errorMonitor->VerifyFound();

    // Check debug buffer to cross check that every expected shader invocation happened
    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], (2 + 0) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[3], (1 + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[4], (1 + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[5], (1 + 2) * frames_count);
    debug_buffer.Memory().Unmap();
}

TEST_F(NegativeDebugPrintfRayTracing, OneMultiEntryPointsShader2CmdTraceRaysIndirect) {
    TEST_DESCRIPTION(
        "Test debug printf in a multi entry points shader. 2 ray generation shaders, 2 miss shaders, 2 closest hit shaders."
        "Trace rays using vkCmdTraceRaysIndirect2KHR");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    std::shared_ptr<vkt::as::BuildGeometryInfoKHR> cube_blas;
    vkt::as::BuildGeometryInfoKHR tlas = GetCubesTLAS(cube_blas);

    // Buffer used to count invocations for the 2 * 3 shaders
    vkt::Buffer debug_buffer(*m_device, 2 * 3 * sizeof(uint32_t),
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer.handle(), debug_buffer.handle(), 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vkt::rt::Pipeline pipeline(*this, m_device);

    pipeline.AddSpirvRayGenShader(GetSlangShader1(), "rayGenShader");
    pipeline.AddSpirvRayGenShader(GetSlangShader1(), "rayGenShader2");
    pipeline.AddSpirvMissShader(GetSlangShader1(), "missShader");
    pipeline.AddSpirvMissShader(GetSlangShader1(), "missShader2");
    pipeline.AddSpirvClosestHitShader(GetSlangShader1(), "closestHitShader");
    pipeline.AddSpirvClosestHitShader(GetSlangShader1(), "closestHitShader2");

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.Build();

    vkt::Buffer sbt_ray_gen_1 = pipeline.GetTraceRaysSbtIndirectBuffer(0, 1, 1, 1);
    vkt::Buffer sbt_ray_gen_2 = pipeline.GetTraceRaysSbtIndirectBuffer(1, 1, 1, 1);

    uint32_t frames_count = 42;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(),
                                  0, 1, &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());

        // Invoke ray gen shader 1
        vk::CmdTraceRaysIndirect2KHR(m_command_buffer.handle(), sbt_ray_gen_1.Address());

        // Invoke ray gen shader 2
        vk::CmdTraceRaysIndirect2KHR(m_command_buffer.handle(), sbt_ray_gen_2.Address());

        m_command_buffer.End();
        m_errorMonitor->SetDesiredInfo("In Raygen 1");
        m_errorMonitor->SetDesiredInfo("In Raygen 2");
        m_errorMonitor->SetDesiredInfo("In Miss 1", 2);
        m_errorMonitor->SetDesiredInfo("In Miss 2", 2);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 1", 2);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 2", 3);
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }

    // Check debug buffer to cross check that every expected shader invocation happened
    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], (2 + 0) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[3], (1 + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[4], (1 + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[5], (1 + 2) * frames_count);
    debug_buffer.Memory().Unmap();
}

TEST_F(NegativeDebugPrintfRayTracing, OneMultiEntryPointsShader2CmdTraceRaysIndirectDeferredBuild) {
    TEST_DESCRIPTION(
        "Test debug printf in a multi entry points shader. 2 ray generation shaders, 2 miss shaders, 2 closest hit shaders."
        "Trace rays using vkCmdTraceRaysIndirect2KHR");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::rayTracingPipelineTraceRaysIndirect2);
    RETURN_IF_SKIP(InitFrameworkWithPrintfBufferSize(1024 * 1024));
    RETURN_IF_SKIP(InitState());

    std::shared_ptr<vkt::as::BuildGeometryInfoKHR> cube_blas;
    vkt::as::BuildGeometryInfoKHR tlas = GetCubesTLAS(cube_blas);

    // Buffer used to count invocations for the 2 * 3 shaders
    vkt::Buffer debug_buffer(*m_device, 2 * 3 * sizeof(uint32_t),
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, kHostVisibleMemProps);
    m_command_buffer.Begin();
    vk::CmdFillBuffer(m_command_buffer.handle(), debug_buffer.handle(), 0, debug_buffer.CreateInfo().size, 0);
    m_command_buffer.End();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    vkt::rt::Pipeline pipeline(*this, m_device);

    pipeline.AddSpirvRayGenShader(GetSlangShader1(), "rayGenShader");
    pipeline.AddSpirvRayGenShader(GetSlangShader1(), "rayGenShader2");
    pipeline.AddSpirvMissShader(GetSlangShader1(), "missShader");
    pipeline.AddSpirvMissShader(GetSlangShader1(), "missShader2");
    pipeline.AddSpirvClosestHitShader(GetSlangShader1(), "closestHitShader");
    pipeline.AddSpirvClosestHitShader(GetSlangShader1(), "closestHitShader2");

    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
    pipeline.AddBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    pipeline.CreateDescriptorSet();
    pipeline.GetDescriptorSet().WriteDescriptorAccelStruct(0, 1, &tlas.GetDstAS()->handle());
    pipeline.GetDescriptorSet().WriteDescriptorBufferInfo(1, debug_buffer.handle(), 0, VK_WHOLE_SIZE,
                                                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipeline.GetDescriptorSet().UpdateDescriptorSets();

    pipeline.DeferBuild();
    pipeline.Build();

    const uint32_t ray_gen_1_width = 2;
    const uint32_t ray_gen_1_height = 2;
    const uint32_t ray_gen_1_depth = 1;
    const uint32_t ray_gen_1_rays_count = ray_gen_1_width * ray_gen_1_height * ray_gen_1_depth;
    vkt::Buffer sbt_ray_gen_1 = pipeline.GetTraceRaysSbtIndirectBuffer(0, ray_gen_1_width, ray_gen_1_height, ray_gen_1_depth);
    vkt::Buffer sbt_ray_gen_2 = pipeline.GetTraceRaysSbtIndirectBuffer(1, 1, 1, 1);

    uint32_t frames_count = 1;
    for (uint32_t frame = 0; frame < frames_count; ++frame) {
        m_command_buffer.Begin();
        vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.GetPipelineLayout(),
                                  0, 1, &pipeline.GetDescriptorSet().set_, 0, nullptr);
        vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.Handle());

        // Invoke ray gen shader 1
        vk::CmdTraceRaysIndirect2KHR(m_command_buffer.handle(), sbt_ray_gen_1.Address());

        // Invoke ray gen shader 2
        vk::CmdTraceRaysIndirect2KHR(m_command_buffer.handle(), sbt_ray_gen_2.Address());

        m_command_buffer.End();
        m_errorMonitor->SetDesiredInfo("In Raygen 1", ray_gen_1_rays_count);
        m_errorMonitor->SetDesiredInfo("In Raygen 2");
        m_errorMonitor->SetDesiredInfo("In Miss 1", 2 * ray_gen_1_rays_count + 0);
        m_errorMonitor->SetDesiredInfo("In Miss 2", 1 * ray_gen_1_rays_count + 1);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 1", 1 * ray_gen_1_rays_count + 1);
        m_errorMonitor->SetDesiredInfo("In Closest Hit 2", 1 * ray_gen_1_rays_count + 2);
        m_default_queue->Submit(m_command_buffer);
        m_default_queue->Wait();
    }
    m_errorMonitor->VerifyFound();

    // Check debug buffer to cross check that every expected shader invocation happened
    auto debug_buffer_ptr = static_cast<uint32_t*>(debug_buffer.Memory().Map());
    ASSERT_EQ(debug_buffer_ptr[0], 1 * ray_gen_1_rays_count * frames_count);
    ASSERT_EQ(debug_buffer_ptr[1], 1 * frames_count);
    ASSERT_EQ(debug_buffer_ptr[2], (2 * ray_gen_1_rays_count + 0) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[3], (1 * ray_gen_1_rays_count + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[4], (1 * ray_gen_1_rays_count + 1) * frames_count);
    ASSERT_EQ(debug_buffer_ptr[5], (1 * ray_gen_1_rays_count + 2) * frames_count);
    debug_buffer.Memory().Unmap();
}
