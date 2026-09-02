// RUN: %dxc -T ps_6_0 -E main -fvk-bind-globals 1 2 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_Globals DescriptorSet 2
// CHECK: OpDecorate %_Globals Binding 1

int globalInteger;
float4 globalFloat4;

float4 main() : SV_Target {
  return (globalInteger + globalFloat4.z).xxxx;
}

