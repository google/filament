//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> ldexp_7485ce() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  int3 arg_1 = (int(1)).xxx;
  vector<float16_t, 3> res = ldexp(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, ldexp_7485ce());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> ldexp_7485ce() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  int3 arg_1 = (int(1)).xxx;
  vector<float16_t, 3> res = ldexp(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, ldexp_7485ce());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  vector<float16_t, 3> prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation vector<float16_t, 3> VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


vector<float16_t, 3> ldexp_7485ce() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  int3 arg_1 = (int(1)).xxx;
  vector<float16_t, 3> res = ldexp(arg_0, arg_1);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = ldexp_7485ce();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

