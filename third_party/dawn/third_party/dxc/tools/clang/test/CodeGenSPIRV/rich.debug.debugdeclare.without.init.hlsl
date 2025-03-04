// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

// CHECK: %i = OpFunctionParameter %_ptr_Function_PS_INPUT
// CHECK: DebugDeclare {{%[0-9]+}} %i
// CHECK: %ps_output = OpVariable %_ptr_Function_PS_OUTPUT Function
// CHECK: %c = OpVariable %_ptr_Function_v4float Function
// CHECK: DebugDeclare {{%[0-9]+}} %ps_output
// CHECK: DebugDeclare {{%[0-9]+}} %c

Texture2D g_tColor;

SamplerState g_sAniso;

struct PS_INPUT {
  float2 vTextureCoords2 : TEXCOORD2;
  float2 vTextureCoords3 : TEXCOORD3;
};

struct PS_OUTPUT {
  float4 vColor : SV_Target0;
};

PS_OUTPUT main(PS_INPUT i) {
  PS_OUTPUT ps_output;
  float4 c;
  c = g_tColor.Sample(g_sAniso, i.vTextureCoords2.xy);
  c += g_tColor.Sample(g_sAniso, i.vTextureCoords3.xy);
  ps_output.vColor = c;
  return ps_output;
}
