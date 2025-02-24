//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleBaseClampToEdge_9ca02c() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(v.x, v.y);
  float2 v_1 = ((0.5f).xx / float2(v));
  float4 res = arg_0.SampleLevel(arg_1, clamp((1.0f).xx, v_1, ((1.0f).xx - v_1)), 0.0f);
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
  uint2 v = (0u).xx;
  arg_0.GetDimensions(v.x, v.y);
  float2 v_1 = ((0.5f).xx / float2(v));
  float4 res = arg_0.SampleLevel(arg_1, clamp((1.0f).xx, v_1, ((1.0f).xx - v_1)), 0.0f);
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
  uint2 v = (0u).xx;
  arg_0.GetDimensions(v.x, v.y);
  float2 v_1 = ((0.5f).xx / float2(v));
  float4 res = arg_0.SampleLevel(arg_1, clamp((1.0f).xx, v_1, ((1.0f).xx - v_1)), 0.0f);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = textureSampleBaseClampToEdge_9ca02c();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

