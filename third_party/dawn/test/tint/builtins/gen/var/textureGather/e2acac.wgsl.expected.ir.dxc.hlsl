//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray<uint4> arg_1 : register(t1, space1);
SamplerState arg_2 : register(s2, space1);
uint4 textureGather_e2acac() {
  float3 arg_3 = (1.0f).xxx;
  uint arg_4 = 1u;
  float3 v = arg_3;
  uint4 res = arg_1.GatherGreen(arg_2, float4(v, float(arg_4)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureGather_e2acac());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray<uint4> arg_1 : register(t1, space1);
SamplerState arg_2 : register(s2, space1);
uint4 textureGather_e2acac() {
  float3 arg_3 = (1.0f).xxx;
  uint arg_4 = 1u;
  float3 v = arg_3;
  uint4 res = arg_1.GatherGreen(arg_2, float4(v, float(arg_4)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureGather_e2acac());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


TextureCubeArray<uint4> arg_1 : register(t1, space1);
SamplerState arg_2 : register(s2, space1);
uint4 textureGather_e2acac() {
  float3 arg_3 = (1.0f).xxx;
  uint arg_4 = 1u;
  float3 v = arg_3;
  uint4 res = arg_1.GatherGreen(arg_2, float4(v, float(arg_4)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = textureGather_e2acac();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

