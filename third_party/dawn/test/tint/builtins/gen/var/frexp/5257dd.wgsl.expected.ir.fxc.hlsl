SKIP: INVALID

struct frexp_result_f16 {
  float16_t fract;
  int exp;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void frexp_5257dd() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t v = arg_0;
  float16_t v_1 = float16_t(0.0h);
  float16_t v_2 = frexp(v, v_1);
  float16_t v_3 = float16_t(sign(v));
  v_1 = (v_3 * v_1);
  frexp_result_f16 res = {v_2, int(v_1)};
}

void fragment_main() {
  frexp_5257dd();
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_5257dd();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  frexp_5257dd();
  VertexOutput v_4 = tint_symbol;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.pos};
  return v_6;
}

FXC validation failure:
<scrubbed_path>(2,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
