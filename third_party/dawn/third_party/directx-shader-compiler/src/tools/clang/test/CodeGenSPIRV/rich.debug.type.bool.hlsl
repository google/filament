// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK: [[typeName:%[0-9]+]] = OpString "bool"

float4 main(float4 color : COLOR) : SV_TARGET {
  // CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeBasic [[typeName]] %uint_32 Boolean
  bool condition = false;
  return color;
}

