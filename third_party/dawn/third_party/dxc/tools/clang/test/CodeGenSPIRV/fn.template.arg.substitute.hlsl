// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl %s -spirv | FileCheck %s

template <bool B1>
bool fnTemplate(bool B2) {
  return !B1 && B2;
}

// CHECK:  {{%[0-9]+}} = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
// CHECK:  {{%[0-9]+}} = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK: %main = OpFunction %void None {{%[0-9]+}}
float4 main(int val : A) : SV_Target {
  return fnTemplate<false>(val != 0) ? (float4)1 : (float4)0;
}
