// RUN: %dxc -T vs_6_6 -spirv -fvk-use-dx-layout -fspv-target-env=vulkan1.3  -fspv-extension=SPV_KHR_non_semantic_info -fspv-debug=vulkan-with-source -fcgl %s | FileCheck %s

// CHECK: [[dbg_ty_name:%[0-9]+]] = OpString "type.cbuff"
// CHECK:    [[dbg_name:%[0-9]+]] = OpString "cbuff"

cbuffer cbuff
{
    float4 a;
};
// CHECK: [[type_cbuff:%[0-9a-zA-Z_]+]] = OpTypeStruct %v4float
// CHECK: [[ptr_cbuff:%[0-9a-zA-Z_]+]]  = OpTypePointer Uniform [[type_cbuff]]
// CHECK: [[cbuff:%[0-9a-zA-Z_]+]]      = OpVariable [[ptr_cbuff]] Uniform


// CHECK:  [[dbg_member:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugTypeMember {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} %uint_8 %uint_12 %uint_0 %uint_128 %uint_3
// CHECK:      [[dbg_ty:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugTypeComposite [[dbg_ty_name]] %uint_1 {{%[0-9]+}} %uint_6 %uint_9 {{%[0-9]+}} [[dbg_ty_name]] %uint_128 %uint_3 [[dbg_member]]
// CHECK:                           OpExtInst %void {{%[0-9]+}} DebugGlobalVariable [[dbg_name]] [[dbg_ty]] {{%[0-9]+}} %uint_6 %uint_9 {{%[0-9]+}} [[dbg_name]] %cbuff %uint_8


float4 main() : SV_Position
{
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float [[cbuff]] %int_0
// CHECK:                   OpLoad %v4float [[ptr]]
    return a;
}
