//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSampleLevel_f3b2c8() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float2 v = arg_2;
  float res = arg_0.SampleLevel(arg_1, v, float(arg_3), (int(1)).xx).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleLevel_f3b2c8()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSampleLevel_f3b2c8() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float2 v = arg_2;
  float res = arg_0.SampleLevel(arg_1, v, float(arg_3), (int(1)).xx).x;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureSampleLevel_f3b2c8()));
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


Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSampleLevel_f3b2c8() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float2 v = arg_2;
  float res = arg_0.SampleLevel(arg_1, v, float(arg_3), (int(1)).xx).x;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = textureSampleLevel_f3b2c8();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

