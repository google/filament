// RUN: %dxc -T ps_6_2 -E main -fspv-debug=rich -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK:        [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK:  [[float64Name:%[0-9]+]] = OpString "float64_t"
// CHECK:    [[floatName:%[0-9]+]] = OpString "float"
// CHECK:  [[float16Name:%[0-9]+]] = OpString "float16_t"
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeBasic [[float64Name]] %uint_64 Float
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeBasic [[floatName]] %uint_32 Float
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeBasic [[float16Name]] %uint_16 Float

float4 main(float4 color : COLOR) : SV_TARGET {
  float a = 0;
  float16_t b = 0;
  float64_t c = 0;
  return color;
}
