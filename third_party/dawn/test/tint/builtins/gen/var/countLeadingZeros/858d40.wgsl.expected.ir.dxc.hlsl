//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 countLeadingZeros_858d40() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v <= (65535u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = ((((v << v_1) <= (16777215u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_3 = (((((v << v_1) << v_2) <= (268435455u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_4 = ((((((v << v_1) << v_2) << v_3) <= (1073741823u).xx)) ? ((2u).xx) : ((0u).xx));
  uint2 v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= (2147483647u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == (0u).xx)) ? ((1u).xx) : ((0u).xx));
  int2 res = asint(((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(countLeadingZeros_858d40()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 countLeadingZeros_858d40() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v <= (65535u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = ((((v << v_1) <= (16777215u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_3 = (((((v << v_1) << v_2) <= (268435455u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_4 = ((((((v << v_1) << v_2) << v_3) <= (1073741823u).xx)) ? ((2u).xx) : ((0u).xx));
  uint2 v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= (2147483647u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == (0u).xx)) ? ((1u).xx) : ((0u).xx));
  int2 res = asint(((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(countLeadingZeros_858d40()));
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


int2 countLeadingZeros_858d40() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v <= (65535u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = ((((v << v_1) <= (16777215u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_3 = (((((v << v_1) << v_2) <= (268435455u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_4 = ((((((v << v_1) << v_2) << v_3) <= (1073741823u).xx)) ? ((2u).xx) : ((0u).xx));
  uint2 v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= (2147483647u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == (0u).xx)) ? ((1u).xx) : ((0u).xx));
  int2 res = asint(((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_7 = (VertexOutput)0;
  v_7.pos = (0.0f).xxxx;
  v_7.prevent_dce = countLeadingZeros_858d40();
  VertexOutput v_8 = v_7;
  return v_8;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_9 = vertex_main_inner();
  vertex_main_outputs v_10 = {v_9.prevent_dce, v_9.pos};
  return v_10;
}

