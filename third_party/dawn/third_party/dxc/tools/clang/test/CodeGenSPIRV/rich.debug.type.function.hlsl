// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// CHECK:    [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK:  [[int:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Signed
// CHECK:  [[float:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeBasic {{%[0-9]+}} %uint_32 Float
//
// Debug function type
// TODO: FlagIsPublic (3u) is shown as FlagIsProtected|FlagIsPrivate.
//
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeFunction FlagIsProtected|FlagIsPrivate %void [[int]] [[float]]

// CHECK: [[float4:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeVector [[float]] 4
//
// Debug function type
// TODO: FlagIsPublic (3u) is shown as FlagIsProtected|FlagIsPrivate.
//
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugTypeFunction FlagIsProtected|FlagIsPrivate [[float4]] [[float4]]

void foo(int x, float y) {
  x = x + y;
}

float4 main(float4 color : COLOR) : SV_TARGET {
  bool condition = false;
  foo(1, color.x);
  return color;
}

