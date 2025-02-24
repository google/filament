//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 firstLeadingBit_000ff3() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = ((((v & (4294901760u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((16u).xxxx));
  uint4 v_2 = (((((v >> v_1) & (65280u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((8u).xxxx));
  uint4 v_3 = ((((((v >> v_1) >> v_2) & (240u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((4u).xxxx));
  uint4 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((2u).xxxx));
  uint4 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxxx)) ? ((4294967295u).xxxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((1u).xxxx))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, firstLeadingBit_000ff3());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 firstLeadingBit_000ff3() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = ((((v & (4294901760u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((16u).xxxx));
  uint4 v_2 = (((((v >> v_1) & (65280u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((8u).xxxx));
  uint4 v_3 = ((((((v >> v_1) >> v_2) & (240u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((4u).xxxx));
  uint4 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((2u).xxxx));
  uint4 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxxx)) ? ((4294967295u).xxxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((1u).xxxx))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, firstLeadingBit_000ff3());
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


uint4 firstLeadingBit_000ff3() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = ((((v & (4294901760u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((16u).xxxx));
  uint4 v_2 = (((((v >> v_1) & (65280u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((8u).xxxx));
  uint4 v_3 = ((((((v >> v_1) >> v_2) & (240u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((4u).xxxx));
  uint4 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((2u).xxxx));
  uint4 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxxx)) ? ((4294967295u).xxxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((1u).xxxx))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = firstLeadingBit_000ff3();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

