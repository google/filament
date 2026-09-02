// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

float4 main(float4 input : A) : SV_Target {
// CHECK:       [[vec:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
// CHECK:                      OpStore %var1 [[vec]]
	float4 var1 = float4(1,2,3,4);

// CHECK-NEXT: [[vec1:%[0-9]+]] = OpLoad %v4float %var1
// CHECK-NEXT:                 OpStore %var2 [[vec1]]
	float4x1 var2 = var1;

// CHECK-NEXT:      [[vec2:%[0-9]+]] = OpLoad %v4float %input
// CHECK-NEXT:                 OpStore %var3 [[vec2]]
	float1x4 var3 = input;

// CHECK-NEXT: [[vec3:%[0-9]+]] = OpLoad %v4float %input
// CHECK-NEXT:                 OpStore %var4 [[vec3]]
	float4x1 var4 = input;

    return input;
}
