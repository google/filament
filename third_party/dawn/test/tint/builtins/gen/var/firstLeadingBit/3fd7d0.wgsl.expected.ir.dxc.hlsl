//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 firstLeadingBit_3fd7d0() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = ((((v & (4294901760u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((16u).xxx));
  uint3 v_2 = (((((v >> v_1) & (65280u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((8u).xxx));
  uint3 v_3 = ((((((v >> v_1) >> v_2) & (240u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((4u).xxx));
  uint3 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((2u).xxx));
  uint3 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((1u).xxx))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, firstLeadingBit_3fd7d0());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 firstLeadingBit_3fd7d0() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = ((((v & (4294901760u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((16u).xxx));
  uint3 v_2 = (((((v >> v_1) & (65280u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((8u).xxx));
  uint3 v_3 = ((((((v >> v_1) >> v_2) & (240u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((4u).xxx));
  uint3 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((2u).xxx));
  uint3 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((1u).xxx))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, firstLeadingBit_3fd7d0());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint3 firstLeadingBit_3fd7d0() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = ((((v & (4294901760u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((16u).xxx));
  uint3 v_2 = (((((v >> v_1) & (65280u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((8u).xxx));
  uint3 v_3 = ((((((v >> v_1) >> v_2) & (240u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((4u).xxx));
  uint3 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((2u).xxx));
  uint3 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((1u).xxx))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = firstLeadingBit_3fd7d0();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

