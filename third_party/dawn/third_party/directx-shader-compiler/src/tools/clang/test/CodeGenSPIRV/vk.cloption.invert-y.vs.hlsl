// RUN: %dxc -T vs_6_0 -E main -fvk-invert-y -fcgl  %s -spirv | FileCheck %s

float4 main(float4 a : A) : SV_Position {
    return a;
}

// CHECK:         [[a:%[0-9]+]] = OpFunctionCall %v4float %src_main %param_var_a
// CHECK-NEXT: [[oldY:%[0-9]+]] = OpCompositeExtract %float [[a]] 1
// CHECK-NEXT: [[newY:%[0-9]+]] = OpFNegate %float [[oldY]]
// CHECK-NEXT:  [[pos:%[0-9]+]] = OpCompositeInsert %v4float [[newY]] [[a]] 1
// CHECK-NEXT:                 OpStore %gl_Position [[pos]]
