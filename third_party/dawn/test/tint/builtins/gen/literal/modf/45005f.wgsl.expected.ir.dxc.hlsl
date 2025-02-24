//
// fragment_main
//
struct modf_result_vec3_f16 {
  vector<float16_t, 3> fract;
  vector<float16_t, 3> whole;
};


void modf_45005f() {
  modf_result_vec3_f16 res = {(float16_t(-0.5h)).xxx, (float16_t(-1.0h)).xxx};
}

void fragment_main() {
  modf_45005f();
}

//
// compute_main
//
struct modf_result_vec3_f16 {
  vector<float16_t, 3> fract;
  vector<float16_t, 3> whole;
};


void modf_45005f() {
  modf_result_vec3_f16 res = {(float16_t(-0.5h)).xxx, (float16_t(-1.0h)).xxx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_45005f();
}

//
// vertex_main
//
struct modf_result_vec3_f16 {
  vector<float16_t, 3> fract;
  vector<float16_t, 3> whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_45005f() {
  modf_result_vec3_f16 res = {(float16_t(-0.5h)).xxx, (float16_t(-1.0h)).xxx};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  modf_45005f();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

