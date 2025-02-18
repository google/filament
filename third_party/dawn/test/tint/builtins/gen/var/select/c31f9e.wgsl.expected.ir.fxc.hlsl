//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int select_c31f9e() {
  bool arg_0 = true;
  bool arg_1 = true;
  bool arg_2 = true;
  bool res = ((arg_2) ? (arg_1) : (arg_0));
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(select_c31f9e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int select_c31f9e() {
  bool arg_0 = true;
  bool arg_1 = true;
  bool arg_2 = true;
  bool res = ((arg_2) ? (arg_1) : (arg_0));
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(select_c31f9e()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int select_c31f9e() {
  bool arg_0 = true;
  bool arg_1 = true;
  bool arg_2 = true;
  bool res = ((arg_2) ? (arg_1) : (arg_0));
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = select_c31f9e();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

