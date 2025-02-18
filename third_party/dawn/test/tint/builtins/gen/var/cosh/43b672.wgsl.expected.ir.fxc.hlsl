SKIP: INVALID

struct VertexOutput {
  float4 pos;
  vector<float16_t, 2> prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation vector<float16_t, 2> VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> cosh_43b672() {
  vector<float16_t, 2> arg_0 = (float16_t(0.0h)).xx;
  vector<float16_t, 2> res = cosh(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, cosh_43b672());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, cosh_43b672());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = cosh_43b672();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  VertexOutput v_2 = v_1;
  VertexOutput v_3 = v_1;
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_2.pos};
  return v_4;
}

FXC validation failure:
<scrubbed_path>(3,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
