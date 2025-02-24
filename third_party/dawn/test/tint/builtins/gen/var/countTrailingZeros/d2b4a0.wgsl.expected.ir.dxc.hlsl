//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 countTrailingZeros_d2b4a0() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = ((((v & (65535u).xxxx) == (0u).xxxx)) ? ((16u).xxxx) : ((0u).xxxx));
  uint4 v_2 = (((((v >> v_1) & (255u).xxxx) == (0u).xxxx)) ? ((8u).xxxx) : ((0u).xxxx));
  uint4 v_3 = ((((((v >> v_1) >> v_2) & (15u).xxxx) == (0u).xxxx)) ? ((4u).xxxx) : ((0u).xxxx));
  uint4 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xxxx) == (0u).xxxx)) ? ((2u).xxxx) : ((0u).xxxx));
  uint4 v_5 = ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xxxx) == (0u).xxxx)) ? ((1u).xxxx) : ((0u).xxxx));
  uint4 res = ((v_1 | (v_2 | (v_3 | (v_4 | v_5)))) + (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxxx)) ? ((1u).xxxx) : ((0u).xxxx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, countTrailingZeros_d2b4a0());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 countTrailingZeros_d2b4a0() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = ((((v & (65535u).xxxx) == (0u).xxxx)) ? ((16u).xxxx) : ((0u).xxxx));
  uint4 v_2 = (((((v >> v_1) & (255u).xxxx) == (0u).xxxx)) ? ((8u).xxxx) : ((0u).xxxx));
  uint4 v_3 = ((((((v >> v_1) >> v_2) & (15u).xxxx) == (0u).xxxx)) ? ((4u).xxxx) : ((0u).xxxx));
  uint4 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xxxx) == (0u).xxxx)) ? ((2u).xxxx) : ((0u).xxxx));
  uint4 v_5 = ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xxxx) == (0u).xxxx)) ? ((1u).xxxx) : ((0u).xxxx));
  uint4 res = ((v_1 | (v_2 | (v_3 | (v_4 | v_5)))) + (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxxx)) ? ((1u).xxxx) : ((0u).xxxx)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, countTrailingZeros_d2b4a0());
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


uint4 countTrailingZeros_d2b4a0() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = ((((v & (65535u).xxxx) == (0u).xxxx)) ? ((16u).xxxx) : ((0u).xxxx));
  uint4 v_2 = (((((v >> v_1) & (255u).xxxx) == (0u).xxxx)) ? ((8u).xxxx) : ((0u).xxxx));
  uint4 v_3 = ((((((v >> v_1) >> v_2) & (15u).xxxx) == (0u).xxxx)) ? ((4u).xxxx) : ((0u).xxxx));
  uint4 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xxxx) == (0u).xxxx)) ? ((2u).xxxx) : ((0u).xxxx));
  uint4 v_5 = ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xxxx) == (0u).xxxx)) ? ((1u).xxxx) : ((0u).xxxx));
  uint4 res = ((v_1 | (v_2 | (v_3 | (v_4 | v_5)))) + (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxxx)) ? ((1u).xxxx) : ((0u).xxxx)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_6 = (VertexOutput)0;
  v_6.pos = (0.0f).xxxx;
  v_6.prevent_dce = countTrailingZeros_d2b4a0();
  VertexOutput v_7 = v_6;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

