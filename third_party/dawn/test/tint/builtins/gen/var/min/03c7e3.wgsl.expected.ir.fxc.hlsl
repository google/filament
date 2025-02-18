//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 min_03c7e3() {
  int2 arg_0 = (int(1)).xx;
  int2 arg_1 = (int(1)).xx;
  int2 res = min(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(min_03c7e3()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 min_03c7e3() {
  int2 arg_0 = (int(1)).xx;
  int2 arg_1 = (int(1)).xx;
  int2 res = min(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(min_03c7e3()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int2 min_03c7e3() {
  int2 arg_0 = (int(1)).xx;
  int2 arg_1 = (int(1)).xx;
  int2 res = min(arg_0, arg_1);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = min_03c7e3();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

