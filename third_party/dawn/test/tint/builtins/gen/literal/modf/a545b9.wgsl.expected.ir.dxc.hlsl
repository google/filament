//
// fragment_main
//
struct modf_result_vec2_f16 {
  vector<float16_t, 2> fract;
  vector<float16_t, 2> whole;
};


void modf_a545b9() {
  modf_result_vec2_f16 res = {(float16_t(-0.5h)).xx, (float16_t(-1.0h)).xx};
}

void fragment_main() {
  modf_a545b9();
}

//
// compute_main
//
struct modf_result_vec2_f16 {
  vector<float16_t, 2> fract;
  vector<float16_t, 2> whole;
};


void modf_a545b9() {
  modf_result_vec2_f16 res = {(float16_t(-0.5h)).xx, (float16_t(-1.0h)).xx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_a545b9();
}

//
// vertex_main
//
struct modf_result_vec2_f16 {
  vector<float16_t, 2> fract;
  vector<float16_t, 2> whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_a545b9() {
  modf_result_vec2_f16 res = {(float16_t(-0.5h)).xx, (float16_t(-1.0h)).xx};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  modf_a545b9();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

