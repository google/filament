SKIP: INVALID

struct frexp_result_vec4_f16 {
  vector<float16_t, 4> fract;
  int4 exp;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void frexp_3dd21e() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  vector<float16_t, 4> v = arg_0;
  vector<float16_t, 4> v_1 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> v_2 = frexp(v, v_1);
  vector<float16_t, 4> v_3 = vector<float16_t, 4>(sign(v));
  v_1 = (v_3 * v_1);
  frexp_result_vec4_f16 res = {v_2, int4(v_1)};
}

void fragment_main() {
  frexp_3dd21e();
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_3dd21e();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  frexp_3dd21e();
  VertexOutput v_4 = tint_symbol;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.pos};
  return v_6;
}

FXC validation failure:
<scrubbed_path>(2,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
