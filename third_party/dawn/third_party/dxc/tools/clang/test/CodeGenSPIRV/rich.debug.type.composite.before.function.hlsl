// RUN: %dxc -T vs_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// CHECK: [[set:%[0-9]+]] = OpExtInstImport "OpenCL.DebugInfo.100"
// CHECK: [[out:%[0-9]+]] = OpString "VS_OUTPUT"
// CHECK: [[main:%[0-9]+]] = OpString "main"

// CHECK: [[VSOUT:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeComposite [[out]]
// CHECK: [[ty:%[0-9]+]] = OpExtInst %void [[set]] DebugTypeFunction FlagIsProtected|FlagIsPrivate [[VSOUT]]
// CHECK: {{%[0-9]+}} = OpExtInst %void [[set]] DebugFunction [[main]] [[ty]]

struct VS_OUTPUT {
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};

VS_OUTPUT main(float4 pos : POSITION,
               float4 color : COLOR) {
  VS_OUTPUT vout;
  vout.pos = pos;
  vout.color = color;
  return vout;
}
