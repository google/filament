// RUN: %dxc -T ps_6_1 -E main -fspv-target-env=vulkan1.1 -fspv-debug=tool -Zi -fcgl  %s -spirv | FileCheck %s

// No file path
// CHECK-NOT: OpString
// No source code
// CHECK-NOT: float4 main(uint val
// Have tool
// CHECK:     OpModuleProcessed
// No line
// CHECK-NOT: OpLine

float4 main(uint val : A) : SV_Target {
  uint a = reversebits(val);
  return a;
}
