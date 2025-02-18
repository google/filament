//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 unpack4x8unorm_750c74() {
  uint arg_0 = 1u;
  uint v = arg_0;
  float4 res = (float4(uint4((v & 255u), ((v >> 8u) & 255u), ((v >> 16u) & 255u), (v >> 24u))) / 255.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(unpack4x8unorm_750c74()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 unpack4x8unorm_750c74() {
  uint arg_0 = 1u;
  uint v = arg_0;
  float4 res = (float4(uint4((v & 255u), ((v >> 8u) & 255u), ((v >> 16u) & 255u), (v >> 24u))) / 255.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(unpack4x8unorm_750c74()));
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


float4 unpack4x8unorm_750c74() {
  uint arg_0 = 1u;
  uint v = arg_0;
  float4 res = (float4(uint4((v & 255u), ((v >> 8u) & 255u), ((v >> 16u) & 255u), (v >> 24u))) / 255.0f);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = unpack4x8unorm_750c74();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

