// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// CHECK: [[dbg_info_none:%[0-9]+]] = OpExtInst %void [[ext:%[0-9]+]] DebugInfoNone
// CHECK: [[ty:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite {{%[0-9]+}} Class {{%[0-9]+}} 0 0 {{%[0-9]+}} {{%[0-9]+}} [[dbg_info_none]]
// CHECK: [[param:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeTemplateParameter {{%[0-9]+}} {{%[0-9]+}} [[dbg_info_none]]
// CHECK: OpExtInst %void [[ext]] DebugTypeTemplate [[ty]] [[param]]

Texture2D<float4> tex : register(t0);

float4 main(float4 pos : SV_Position) : SV_Target0
{
  return tex.Load(int3(pos.xy, 0));
}
