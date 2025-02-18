//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSampleLevel_36780e() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = int(1);
  int arg_4 = int(1);
  float2 v = arg_2;
  int v_1 = arg_4;
  float3 v_2 = float3(v, float(arg_3));
  float res = arg_0.SampleLevel(arg_1, v_2, float(v_1), (int(1)).xx).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleLevel_36780e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSampleLevel_36780e() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = int(1);
  int arg_4 = int(1);
  float2 v = arg_2;
  int v_1 = arg_4;
  float3 v_2 = float3(v, float(arg_3));
  float res = arg_0.SampleLevel(arg_1, v_2, float(v_1), (int(1)).xx).x;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureSampleLevel_36780e()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSampleLevel_36780e() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = int(1);
  int arg_4 = int(1);
  float2 v = arg_2;
  int v_1 = arg_4;
  float3 v_2 = float3(v, float(arg_3));
  float res = arg_0.SampleLevel(arg_1, v_2, float(v_1), (int(1)).xx).x;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = textureSampleLevel_36780e();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

