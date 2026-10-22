// RUN: %dxc -T ps_6_2 -E main -fspv-debug=rich -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK:        [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK:    [[intName:%[0-9]+]] = OpString "int"
// CHECK:  [[floatName:%[0-9]+]] = OpString "float"
// CHECK:            %uint_32 = OpConstant %uint 32
// CHECK: [[intType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic [[intName]] %uint_32 Signed
// CHECK:    [[int4:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeVector [[intType]] 4
// CHECK: [[floatType:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic [[floatName]] %uint_32 Float
// CHECK:    [[float3:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeVector [[floatType]] 3

float4 main(float4 color : COLOR) : SV_TARGET {
  float3 b = 0.xxx;
  int4 a = 0.xxxx;

  return color;
}
