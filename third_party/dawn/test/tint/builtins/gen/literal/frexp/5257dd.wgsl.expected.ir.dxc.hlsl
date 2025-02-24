//
// fragment_main
//
struct frexp_result_f16 {
  float16_t fract;
  int exp;
};


void frexp_5257dd() {
  frexp_result_f16 res = {float16_t(0.5h), int(1)};
}

void fragment_main() {
  frexp_5257dd();
}

//
// compute_main
//
struct frexp_result_f16 {
  float16_t fract;
  int exp;
};


void frexp_5257dd() {
  frexp_result_f16 res = {float16_t(0.5h), int(1)};
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_5257dd();
}

//
// vertex_main
//
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

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  frexp_5257dd();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

