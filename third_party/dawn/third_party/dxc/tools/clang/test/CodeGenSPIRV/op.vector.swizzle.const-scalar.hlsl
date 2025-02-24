// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:  [[v4f1:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
// CHECK: [[v4f25:%[0-9]+]] = OpConstantComposite %v4float %float_2_5 %float_2_5 %float_2_5 %float_2_5
// CHECK:  [[v4f0:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK:  [[v3f1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1

float4 main(float input: INPUT) : SV_Target {

// CHECK: OpStore %a [[v4f1]]
  float4 a = (1).xxxx;

// CHECK: OpStore %b [[v4f25]]
  float4 b = (2.5).xxxx;

// CHECK: OpStore %c [[v4f0]]
  float4 c = (false).xxxx;

// CHECK:       [[cc:%[0-9]+]] = OpCompositeConstruct %v2float %float_0 %float_0
// CHECK-NEXT: [[cc0:%[0-9]+]] = OpCompositeExtract %float [[cc]] 0
// CHECK-NEXT: [[cc1:%[0-9]+]] = OpCompositeExtract %float [[cc]] 1
// CHECK-NEXT: [[rhs:%[0-9]+]] = OpCompositeConstruct %v4float {{%[0-9]+}} {{%[0-9]+}} [[cc0]] [[cc1]]
// CHECK-NEXT:                OpStore %d [[rhs]]
  float4 d = float4(input, input, 0.0.xx);

// CHECK:        [[e:%[0-9]+]] = OpLoad %v4float %e
// CHECK-NEXT: [[val:%[0-9]+]] = OpVectorShuffle %v4float [[e]] [[v3f1]] 4 5 6 3
// CHECK-NEXT:                OpStore %e [[val]]
  float4 e;
  (e.xyz)  = 1.0.xxx; // Parentheses

// CHECK: OpReturnValue [[v4f0]]
  return 0.0.xxxx;
}
