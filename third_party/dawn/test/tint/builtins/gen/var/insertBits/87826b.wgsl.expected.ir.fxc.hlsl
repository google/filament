//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 insertBits_87826b() {
  uint3 arg_0 = (1u).xxx;
  uint3 arg_1 = (1u).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint3 v = arg_0;
  uint3 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  uint3 v_6 = (((v_2 < 32u)) ? ((v_1 << uint3((v_2).xxx))) : ((0u).xxx));
  uint3 v_7 = (v_6 & uint3((v_5).xxx));
  uint3 res = (v_7 | (v & uint3((~(v_5)).xxx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, insertBits_87826b());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 insertBits_87826b() {
  uint3 arg_0 = (1u).xxx;
  uint3 arg_1 = (1u).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint3 v = arg_0;
  uint3 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  uint3 v_6 = (((v_2 < 32u)) ? ((v_1 << uint3((v_2).xxx))) : ((0u).xxx));
  uint3 v_7 = (v_6 & uint3((v_5).xxx));
  uint3 res = (v_7 | (v & uint3((~(v_5)).xxx)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, insertBits_87826b());
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


uint3 insertBits_87826b() {
  uint3 arg_0 = (1u).xxx;
  uint3 arg_1 = (1u).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint3 v = arg_0;
  uint3 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  uint3 v_6 = (((v_2 < 32u)) ? ((v_1 << uint3((v_2).xxx))) : ((0u).xxx));
  uint3 v_7 = (v_6 & uint3((v_5).xxx));
  uint3 res = (v_7 | (v & uint3((~(v_5)).xxx)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_8 = (VertexOutput)0;
  v_8.pos = (0.0f).xxxx;
  v_8.prevent_dce = insertBits_87826b();
  VertexOutput v_9 = v_8;
  return v_9;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_10 = vertex_main_inner();
  vertex_main_outputs v_11 = {v_10.prevent_dce, v_10.pos};
  return v_11;
}

