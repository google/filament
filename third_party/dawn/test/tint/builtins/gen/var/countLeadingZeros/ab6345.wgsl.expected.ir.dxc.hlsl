//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 countLeadingZeros_ab6345() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = (((v <= (65535u).xxx)) ? ((16u).xxx) : ((0u).xxx));
  uint3 v_2 = ((((v << v_1) <= (16777215u).xxx)) ? ((8u).xxx) : ((0u).xxx));
  uint3 v_3 = (((((v << v_1) << v_2) <= (268435455u).xxx)) ? ((4u).xxx) : ((0u).xxx));
  uint3 v_4 = ((((((v << v_1) << v_2) << v_3) <= (1073741823u).xxx)) ? ((2u).xxx) : ((0u).xxx));
  uint3 v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= (2147483647u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  uint3 v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == (0u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  uint3 res = ((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, countLeadingZeros_ab6345());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 countLeadingZeros_ab6345() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = (((v <= (65535u).xxx)) ? ((16u).xxx) : ((0u).xxx));
  uint3 v_2 = ((((v << v_1) <= (16777215u).xxx)) ? ((8u).xxx) : ((0u).xxx));
  uint3 v_3 = (((((v << v_1) << v_2) <= (268435455u).xxx)) ? ((4u).xxx) : ((0u).xxx));
  uint3 v_4 = ((((((v << v_1) << v_2) << v_3) <= (1073741823u).xxx)) ? ((2u).xxx) : ((0u).xxx));
  uint3 v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= (2147483647u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  uint3 v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == (0u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  uint3 res = ((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, countLeadingZeros_ab6345());
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


uint3 countLeadingZeros_ab6345() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = (((v <= (65535u).xxx)) ? ((16u).xxx) : ((0u).xxx));
  uint3 v_2 = ((((v << v_1) <= (16777215u).xxx)) ? ((8u).xxx) : ((0u).xxx));
  uint3 v_3 = (((((v << v_1) << v_2) <= (268435455u).xxx)) ? ((4u).xxx) : ((0u).xxx));
  uint3 v_4 = ((((((v << v_1) << v_2) << v_3) <= (1073741823u).xxx)) ? ((2u).xxx) : ((0u).xxx));
  uint3 v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= (2147483647u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  uint3 v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == (0u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  uint3 res = ((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_7 = (VertexOutput)0;
  v_7.pos = (0.0f).xxxx;
  v_7.prevent_dce = countLeadingZeros_ab6345();
  VertexOutput v_8 = v_7;
  return v_8;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_9 = vertex_main_inner();
  vertex_main_outputs v_10 = {v_9.prevent_dce, v_9.pos};
  return v_10;
}

