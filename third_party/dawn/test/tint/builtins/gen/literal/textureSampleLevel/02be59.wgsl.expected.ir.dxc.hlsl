//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSampleLevel_02be59() {
  float res = arg_0.SampleLevel(arg_1, (1.0f).xx, float(int(1))).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleLevel_02be59()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSampleLevel_02be59() {
  float res = arg_0.SampleLevel(arg_1, (1.0f).xx, float(int(1))).x;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureSampleLevel_02be59()));
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
float textureSampleLevel_02be59() {
  float res = arg_0.SampleLevel(arg_1, (1.0f).xx, float(int(1))).x;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = textureSampleLevel_02be59();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

