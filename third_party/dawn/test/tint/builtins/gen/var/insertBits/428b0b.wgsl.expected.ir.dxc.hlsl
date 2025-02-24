//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 insertBits_428b0b() {
  int3 arg_0 = (int(1)).xxx;
  int3 arg_1 = (int(1)).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int3 v = arg_0;
  int3 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  int3 v_6 = (((v_2 < 32u)) ? ((v_1 << uint3((v_2).xxx))) : ((int(0)).xxx));
  int3 v_7 = (v_6 & int3((v_5).xxx));
  int3 res = (v_7 | (v & int3((~(v_5)).xxx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(insertBits_428b0b()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 insertBits_428b0b() {
  int3 arg_0 = (int(1)).xxx;
  int3 arg_1 = (int(1)).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int3 v = arg_0;
  int3 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  int3 v_6 = (((v_2 < 32u)) ? ((v_1 << uint3((v_2).xxx))) : ((int(0)).xxx));
  int3 v_7 = (v_6 & int3((v_5).xxx));
  int3 res = (v_7 | (v & int3((~(v_5)).xxx)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(insertBits_428b0b()));
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


int3 insertBits_428b0b() {
  int3 arg_0 = (int(1)).xxx;
  int3 arg_1 = (int(1)).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int3 v = arg_0;
  int3 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  int3 v_6 = (((v_2 < 32u)) ? ((v_1 << uint3((v_2).xxx))) : ((int(0)).xxx));
  int3 v_7 = (v_6 & int3((v_5).xxx));
  int3 res = (v_7 | (v & int3((~(v_5)).xxx)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_8 = (VertexOutput)0;
  v_8.pos = (0.0f).xxxx;
  v_8.prevent_dce = insertBits_428b0b();
  VertexOutput v_9 = v_8;
  return v_9;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_10 = vertex_main_inner();
  vertex_main_outputs v_11 = {v_10.prevent_dce, v_10.pos};
  return v_11;
}

