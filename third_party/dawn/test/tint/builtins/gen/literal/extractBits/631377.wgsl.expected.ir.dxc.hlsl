//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 extractBits_631377() {
  uint4 res = (0u).xxxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, extractBits_631377());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 extractBits_631377() {
  uint4 res = (0u).xxxx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, extractBits_631377());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint4 extractBits_631377() {
  uint4 res = (0u).xxxx;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = extractBits_631377();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

