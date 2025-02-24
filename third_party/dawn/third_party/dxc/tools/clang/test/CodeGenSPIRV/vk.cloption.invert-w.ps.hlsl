// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-position-w -fcgl  %s -spirv | FileCheck %s

float4 main(float4 pos: SV_Position) : SV_Target {
    return pos;
}

// CHECK:       [[old:%[0-9]+]] = OpLoad %v4float %gl_FragCoord
// CHECK-NEXT: [[oldW:%[0-9]+]] = OpCompositeExtract %float [[old]] 3
// CHECK-NEXT: [[newW:%[0-9]+]] = OpFDiv %float %float_1 [[oldW]]
// CHECK-NEXT:  [[new:%[0-9]+]] = OpCompositeInsert %v4float [[newW]] [[old]] 3
// CHECK-NEXT:                 OpStore %param_var_pos [[new]]
// CHECK-NEXT:                 OpFunctionCall %v4float %src_main %param_var_pos
