SKIP: INVALID

struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_8dbbbf() {
  float16_t arg_0 = float16_t(-1.5h);
  float16_t v = float16_t(0.0h);
  float16_t v_1 = modf(arg_0, v);
  modf_result_f16 res = {v_1, v};
}

void fragment_main() {
  modf_8dbbbf();
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_8dbbbf();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_8dbbbf();
  VertexOutput v_2 = tint_symbol;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.pos};
  return v_4;
}

FXC validation failure:
<scrubbed_path>(2,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
