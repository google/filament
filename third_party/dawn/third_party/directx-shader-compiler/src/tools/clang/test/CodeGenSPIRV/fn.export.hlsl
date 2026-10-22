// RUN: %dxc -T lib_6_3 -fspv-target-env=universal1.5 -fcgl  %s -spirv | FileCheck %s

// CHECK-DAG: OpCapability Shader
// CHECK-DAG: OpCapability Linkage
RWBuffer< float4 > output : register(u1);

// CHECK: OpDecorate %main LinkageAttributes "main" Export
// CHECK: %main = OpFunction %int None
export int main(inout float4 color) {
  output[0] = color;
  return 1;
}
