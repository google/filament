//
// fragment_main
//
struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};


void modf_8dbbbf() {
  modf_result_f16 res = {float16_t(-0.5h), float16_t(-1.0h)};
}

void fragment_main() {
  modf_8dbbbf();
}

//
// compute_main
//
struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};


void modf_8dbbbf() {
  modf_result_f16 res = {float16_t(-0.5h), float16_t(-1.0h)};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_8dbbbf();
}

//
// vertex_main
//
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
  modf_result_f16 res = {float16_t(-0.5h), float16_t(-1.0h)};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  modf_8dbbbf();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

