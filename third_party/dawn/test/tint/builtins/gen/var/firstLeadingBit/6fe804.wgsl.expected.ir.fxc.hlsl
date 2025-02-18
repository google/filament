//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 firstLeadingBit_6fe804() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = ((((v & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_2 = (((((v >> v_1) & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_3 = ((((((v >> v_1) >> v_2) & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  uint2 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xx)) ? ((4294967295u).xx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, firstLeadingBit_6fe804());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 firstLeadingBit_6fe804() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = ((((v & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_2 = (((((v >> v_1) & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_3 = ((((((v >> v_1) >> v_2) & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  uint2 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xx)) ? ((4294967295u).xx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, firstLeadingBit_6fe804());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint2 firstLeadingBit_6fe804() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = ((((v & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_2 = (((((v >> v_1) & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_3 = ((((((v >> v_1) >> v_2) & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  uint2 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xx)) ? ((4294967295u).xx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = firstLeadingBit_6fe804();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

