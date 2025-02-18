//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 firstTrailingBit_7496d6() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = ((((v & (65535u).xxx) == (0u).xxx)) ? ((16u).xxx) : ((0u).xxx));
  uint3 v_2 = (((((v >> v_1) & (255u).xxx) == (0u).xxx)) ? ((8u).xxx) : ((0u).xxx));
  uint3 v_3 = ((((((v >> v_1) >> v_2) & (15u).xxx) == (0u).xxx)) ? ((4u).xxx) : ((0u).xxx));
  uint3 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xxx) == (0u).xxx)) ? ((2u).xxx) : ((0u).xxx));
  int3 res = asint((((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xxx) == (0u).xxx)) ? ((1u).xxx) : ((0u).xxx)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(firstTrailingBit_7496d6()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 firstTrailingBit_7496d6() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = ((((v & (65535u).xxx) == (0u).xxx)) ? ((16u).xxx) : ((0u).xxx));
  uint3 v_2 = (((((v >> v_1) & (255u).xxx) == (0u).xxx)) ? ((8u).xxx) : ((0u).xxx));
  uint3 v_3 = ((((((v >> v_1) >> v_2) & (15u).xxx) == (0u).xxx)) ? ((4u).xxx) : ((0u).xxx));
  uint3 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xxx) == (0u).xxx)) ? ((2u).xxx) : ((0u).xxx));
  int3 res = asint((((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xxx) == (0u).xxx)) ? ((1u).xxx) : ((0u).xxx)))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(firstTrailingBit_7496d6()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int3 firstTrailingBit_7496d6() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = ((((v & (65535u).xxx) == (0u).xxx)) ? ((16u).xxx) : ((0u).xxx));
  uint3 v_2 = (((((v >> v_1) & (255u).xxx) == (0u).xxx)) ? ((8u).xxx) : ((0u).xxx));
  uint3 v_3 = ((((((v >> v_1) >> v_2) & (15u).xxx) == (0u).xxx)) ? ((4u).xxx) : ((0u).xxx));
  uint3 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xxx) == (0u).xxx)) ? ((2u).xxx) : ((0u).xxx));
  int3 res = asint((((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xxx) == (0u).xxx)) ? ((1u).xxx) : ((0u).xxx)))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = firstTrailingBit_7496d6();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

