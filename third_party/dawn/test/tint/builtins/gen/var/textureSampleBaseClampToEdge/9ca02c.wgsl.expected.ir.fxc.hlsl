//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBaseClampToEdge_9ca02c() {
  float2 arg_2 = (1.0f).xx;
  float2 v = arg_2;
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(v_1.x, v_1.y);
  float2 v_2 = ((0.5f).xx / float2(v_1));
  float4 res = arg_0.SampleLevel(arg_1, clamp(v, v_2, ((1.0f).xx - v_2)), 0.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_9ca02c()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBaseClampToEdge_9ca02c() {
  float2 arg_2 = (1.0f).xx;
  float2 v = arg_2;
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(v_1.x, v_1.y);
  float2 v_2 = ((0.5f).xx / float2(v_1));
  float4 res = arg_0.SampleLevel(arg_1, clamp(v, v_2, ((1.0f).xx - v_2)), 0.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_9ca02c()));
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


Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBaseClampToEdge_9ca02c() {
  float2 arg_2 = (1.0f).xx;
  float2 v = arg_2;
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(v_1.x, v_1.y);
  float2 v_2 = ((0.5f).xx / float2(v_1));
  float4 res = arg_0.SampleLevel(arg_1, clamp(v, v_2, ((1.0f).xx - v_2)), 0.0f);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = textureSampleBaseClampToEdge_9ca02c();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

