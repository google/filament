//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 select_a2860e() {
  int4 arg_0 = (int(1)).xxxx;
  int4 arg_1 = (int(1)).xxxx;
  bool4 arg_2 = (true).xxxx;
  int4 res = ((arg_2) ? (arg_1) : (arg_0));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(select_a2860e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 select_a2860e() {
  int4 arg_0 = (int(1)).xxxx;
  int4 arg_1 = (int(1)).xxxx;
  bool4 arg_2 = (true).xxxx;
  int4 res = ((arg_2) ? (arg_1) : (arg_0));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(select_a2860e()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int4 select_a2860e() {
  int4 arg_0 = (int(1)).xxxx;
  int4 arg_1 = (int(1)).xxxx;
  bool4 arg_2 = (true).xxxx;
  int4 res = ((arg_2) ? (arg_1) : (arg_0));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = select_a2860e();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

