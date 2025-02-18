//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 max_b1b73a() {
  uint3 res = (1u).xxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, max_b1b73a());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 max_b1b73a() {
  uint3 res = (1u).xxx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, max_b1b73a());
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


uint3 max_b1b73a() {
  uint3 res = (1u).xxx;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = max_b1b73a();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

