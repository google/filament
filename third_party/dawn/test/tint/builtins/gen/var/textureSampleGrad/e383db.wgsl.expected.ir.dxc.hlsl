//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleGrad_e383db() {
  float3 arg_2 = (1.0f).xxx;
  int arg_3 = int(1);
  float3 arg_4 = (1.0f).xxx;
  float3 arg_5 = (1.0f).xxx;
  float3 v = arg_2;
  float3 v_1 = arg_4;
  float3 v_2 = arg_5;
  float4 res = arg_0.SampleGrad(arg_1, float4(v, float(arg_3)), v_1, v_2);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleGrad_e383db()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleGrad_e383db() {
  float3 arg_2 = (1.0f).xxx;
  int arg_3 = int(1);
  float3 arg_4 = (1.0f).xxx;
  float3 arg_5 = (1.0f).xxx;
  float3 v = arg_2;
  float3 v_1 = arg_4;
  float3 v_2 = arg_5;
  float4 res = arg_0.SampleGrad(arg_1, float4(v, float(arg_3)), v_1, v_2);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureSampleGrad_e383db()));
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


TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleGrad_e383db() {
  float3 arg_2 = (1.0f).xxx;
  int arg_3 = int(1);
  float3 arg_4 = (1.0f).xxx;
  float3 arg_5 = (1.0f).xxx;
  float3 v = arg_2;
  float3 v_1 = arg_4;
  float3 v_2 = arg_5;
  float4 res = arg_0.SampleGrad(arg_1, float4(v, float(arg_3)), v_1, v_2);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = textureSampleGrad_e383db();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

