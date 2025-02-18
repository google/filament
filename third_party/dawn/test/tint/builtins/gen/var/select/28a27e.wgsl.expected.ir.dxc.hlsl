//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 select_28a27e() {
  uint3 arg_0 = (1u).xxx;
  uint3 arg_1 = (1u).xxx;
  bool3 arg_2 = (true).xxx;
  uint3 res = ((arg_2) ? (arg_1) : (arg_0));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, select_28a27e());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 select_28a27e() {
  uint3 arg_0 = (1u).xxx;
  uint3 arg_1 = (1u).xxx;
  bool3 arg_2 = (true).xxx;
  uint3 res = ((arg_2) ? (arg_1) : (arg_0));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, select_28a27e());
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


uint3 select_28a27e() {
  uint3 arg_0 = (1u).xxx;
  uint3 arg_1 = (1u).xxx;
  bool3 arg_2 = (true).xxx;
  uint3 res = ((arg_2) ? (arg_1) : (arg_0));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = select_28a27e();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

