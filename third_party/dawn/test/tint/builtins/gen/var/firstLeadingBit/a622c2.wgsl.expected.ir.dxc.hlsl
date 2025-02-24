//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 firstLeadingBit_a622c2() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v < (2147483648u).xx)) ? (v) : (~(v)));
  uint2 v_2 = ((((v_1 & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_3 = (((((v_1 >> v_2) & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_4 = ((((((v_1 >> v_2) >> v_3) & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  int2 res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == (0u).xx)) ? ((4294967295u).xx) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(firstLeadingBit_a622c2()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 firstLeadingBit_a622c2() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v < (2147483648u).xx)) ? (v) : (~(v)));
  uint2 v_2 = ((((v_1 & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_3 = (((((v_1 >> v_2) & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_4 = ((((((v_1 >> v_2) >> v_3) & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  int2 res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == (0u).xx)) ? ((4294967295u).xx) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx)))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(firstLeadingBit_a622c2()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int2 firstLeadingBit_a622c2() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v < (2147483648u).xx)) ? (v) : (~(v)));
  uint2 v_2 = ((((v_1 & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_3 = (((((v_1 >> v_2) & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_4 = ((((((v_1 >> v_2) >> v_3) & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  int2 res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == (0u).xx)) ? ((4294967295u).xx) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx)))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_6 = (VertexOutput)0;
  v_6.pos = (0.0f).xxxx;
  v_6.prevent_dce = firstLeadingBit_a622c2();
  VertexOutput v_7 = v_6;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

