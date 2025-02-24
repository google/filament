//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint insertBits_e3e3a2() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  uint v_6 = (((v_2 < 32u)) ? ((v_1 << uint(v_2))) : (0u));
  uint v_7 = (v_6 & uint(v_5));
  uint res = (v_7 | (v & uint(~(v_5))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, insertBits_e3e3a2());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint insertBits_e3e3a2() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  uint v_6 = (((v_2 < 32u)) ? ((v_1 << uint(v_2))) : (0u));
  uint v_7 = (v_6 & uint(v_5));
  uint res = (v_7 | (v & uint(~(v_5))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, insertBits_e3e3a2());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint insertBits_e3e3a2() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  uint v_6 = (((v_2 < 32u)) ? ((v_1 << uint(v_2))) : (0u));
  uint v_7 = (v_6 & uint(v_5));
  uint res = (v_7 | (v & uint(~(v_5))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_8 = (VertexOutput)0;
  v_8.pos = (0.0f).xxxx;
  v_8.prevent_dce = insertBits_e3e3a2();
  VertexOutput v_9 = v_8;
  return v_9;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_10 = vertex_main_inner();
  vertex_main_outputs v_11 = {v_10.prevent_dce, v_10.pos};
  return v_11;
}

