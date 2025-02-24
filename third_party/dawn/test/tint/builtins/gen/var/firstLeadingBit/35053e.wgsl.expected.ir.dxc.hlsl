//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 firstLeadingBit_35053e() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = (((v < (2147483648u).xxx)) ? (v) : (~(v)));
  uint3 v_2 = ((((v_1 & (4294901760u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((16u).xxx));
  uint3 v_3 = (((((v_1 >> v_2) & (65280u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((8u).xxx));
  uint3 v_4 = ((((((v_1 >> v_2) >> v_3) & (240u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((4u).xxx));
  uint3 v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & (12u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((2u).xxx));
  int3 res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & (2u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((1u).xxx)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(firstLeadingBit_35053e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 firstLeadingBit_35053e() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = (((v < (2147483648u).xxx)) ? (v) : (~(v)));
  uint3 v_2 = ((((v_1 & (4294901760u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((16u).xxx));
  uint3 v_3 = (((((v_1 >> v_2) & (65280u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((8u).xxx));
  uint3 v_4 = ((((((v_1 >> v_2) >> v_3) & (240u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((4u).xxx));
  uint3 v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & (12u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((2u).xxx));
  int3 res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & (2u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((1u).xxx)))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(firstLeadingBit_35053e()));
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


int3 firstLeadingBit_35053e() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = (((v < (2147483648u).xxx)) ? (v) : (~(v)));
  uint3 v_2 = ((((v_1 & (4294901760u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((16u).xxx));
  uint3 v_3 = (((((v_1 >> v_2) & (65280u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((8u).xxx));
  uint3 v_4 = ((((((v_1 >> v_2) >> v_3) & (240u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((4u).xxx));
  uint3 v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & (12u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((2u).xxx));
  int3 res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & (2u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((1u).xxx)))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_6 = (VertexOutput)0;
  v_6.pos = (0.0f).xxxx;
  v_6.prevent_dce = firstLeadingBit_35053e();
  VertexOutput v_7 = v_6;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

