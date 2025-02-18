//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> asin_b4aced() {
  vector<float16_t, 2> res = (float16_t(0.499755859375h)).xx;
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, asin_b4aced());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> asin_b4aced() {
  vector<float16_t, 2> res = (float16_t(0.499755859375h)).xx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, asin_b4aced());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  vector<float16_t, 2> prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation vector<float16_t, 2> VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


vector<float16_t, 2> asin_b4aced() {
  vector<float16_t, 2> res = (float16_t(0.499755859375h)).xx;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = asin_b4aced();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

