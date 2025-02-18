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
  frexp_result_f16 res = {float16_t(0.5h), int(1)};
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
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

FXC validation failure:
<scrubbed_path>(2,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
