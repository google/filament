//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 firstTrailingBit_50c072() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = ((((v & (65535u).xx) == (0u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = (((((v >> v_1) & (255u).xx) == (0u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_3 = ((((((v >> v_1) >> v_2) & (15u).xx) == (0u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xx) == (0u).xx)) ? ((2u).xx) : ((0u).xx));
  int2 res = asint((((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xx)) ? ((4294967295u).xx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xx) == (0u).xx)) ? ((1u).xx) : ((0u).xx)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(firstTrailingBit_50c072()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 firstTrailingBit_50c072() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = ((((v & (65535u).xx) == (0u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = (((((v >> v_1) & (255u).xx) == (0u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_3 = ((((((v >> v_1) >> v_2) & (15u).xx) == (0u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xx) == (0u).xx)) ? ((2u).xx) : ((0u).xx));
  int2 res = asint((((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xx)) ? ((4294967295u).xx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xx) == (0u).xx)) ? ((1u).xx) : ((0u).xx)))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(firstTrailingBit_50c072()));
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


int2 firstTrailingBit_50c072() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = ((((v & (65535u).xx) == (0u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = (((((v >> v_1) & (255u).xx) == (0u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_3 = ((((((v >> v_1) >> v_2) & (15u).xx) == (0u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xx) == (0u).xx)) ? ((2u).xx) : ((0u).xx));
  int2 res = asint((((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xx)) ? ((4294967295u).xx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xx) == (0u).xx)) ? ((1u).xx) : ((0u).xx)))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = firstTrailingBit_50c072();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

