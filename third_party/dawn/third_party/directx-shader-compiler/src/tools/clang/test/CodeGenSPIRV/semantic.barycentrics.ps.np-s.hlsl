// RUN: %dxc -T ps_6_1 -E main -spirv -Od %s 2>&1 | FileCheck %s

// CHECK:      OpExtension "SPV_KHR_fragment_shader_barycentric"

// CHECK:      OpEntryPoint Fragment
// CHECK-SAME: [[bary:%[0-9]+]]

// CHECK:      OpDecorate [[bary]] BuiltIn BaryCoordNoPerspKHR
// CHECK:      OpDecorate [[bary]] Sample

// CHECK:      [[bary]] = OpVariable %_ptr_Input_v3float Input

float4 main(noperspective sample float3 bary : SV_Barycentrics) : SV_Target {
    return float4(bary, 1.0);
// CHECK:      %param_var_bary = OpVariable %_ptr_Function_v3float Function
// CHECK-NEXT:     [[c2:%[0-9]+]] = OpLoad %v3float [[bary]]
// CHECK-NEXT:      [[x:%[0-9]+]] = OpCompositeExtract %float [[c2]] 0
// CHECK-NEXT:      [[y:%[0-9]+]] = OpCompositeExtract %float [[c2]] 1
// CHECK-NEXT:     [[xy:%[0-9]+]] = OpFAdd %float [[x]] [[y]]
// CHECK-NEXT:      [[z:%[0-9]+]] = OpFSub %float %float_1 [[xy]]
// CHECK-NEXT:     [[c3:%[0-9]+]] = OpCompositeConstruct %v3float [[x]] [[y]] [[z]]
// CHECK-NEXT:                   OpStore %param_var_bary [[c3]]
}
