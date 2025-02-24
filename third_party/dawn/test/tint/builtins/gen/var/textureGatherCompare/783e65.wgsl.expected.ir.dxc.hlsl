//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float4 textureGatherCompare_783e65() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = int(1);
  float arg_4 = 1.0f;
  float2 v = arg_2;
  float v_1 = arg_4;
  float4 res = arg_0.GatherCmp(arg_1, float3(v, float(arg_3)), v_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_783e65()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float4 textureGatherCompare_783e65() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = int(1);
  float arg_4 = 1.0f;
  float2 v = arg_2;
  float v_1 = arg_4;
  float4 res = arg_0.GatherCmp(arg_1, float3(v, float(arg_3)), v_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_783e65()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2DArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float4 textureGatherCompare_783e65() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = int(1);
  float arg_4 = 1.0f;
  float2 v = arg_2;
  float v_1 = arg_4;
  float4 res = arg_0.GatherCmp(arg_1, float3(v, float(arg_3)), v_1);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = textureGatherCompare_783e65();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

