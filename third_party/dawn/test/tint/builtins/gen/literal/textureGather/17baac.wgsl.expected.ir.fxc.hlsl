//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_1 : register(t1, space1);
SamplerState arg_2 : register(s2, space1);
float4 textureGather_17baac() {
  float4 res = arg_1.GatherGreen(arg_2, float3((1.0f).xx, float(1u)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureGather_17baac()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_1 : register(t1, space1);
SamplerState arg_2 : register(s2, space1);
float4 textureGather_17baac() {
  float4 res = arg_1.GatherGreen(arg_2, float3((1.0f).xx, float(1u)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureGather_17baac()));
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


Texture2DArray<float4> arg_1 : register(t1, space1);
SamplerState arg_2 : register(s2, space1);
float4 textureGather_17baac() {
  float4 res = arg_1.GatherGreen(arg_2, float3((1.0f).xx, float(1u)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = textureGather_17baac();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

