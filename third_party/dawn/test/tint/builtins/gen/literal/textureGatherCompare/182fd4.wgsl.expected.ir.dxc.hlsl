//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
TextureCube arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float4 textureGatherCompare_182fd4() {
  float4 res = arg_0.GatherCmp(arg_1, (1.0f).xxx, 1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_182fd4()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
TextureCube arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float4 textureGatherCompare_182fd4() {
  float4 res = arg_0.GatherCmp(arg_1, (1.0f).xxx, 1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureGatherCompare_182fd4()));
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


TextureCube arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float4 textureGatherCompare_182fd4() {
  float4 res = arg_0.GatherCmp(arg_1, (1.0f).xxx, 1.0f);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = textureGatherCompare_182fd4();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

