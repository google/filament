//
// fragment_main
//
struct modf_result_f32 {
  float fract;
  float whole;
};


void modf_bbf7f7() {
  modf_result_f32 res = {-0.5f, -1.0f};
}

void fragment_main() {
  modf_bbf7f7();
}

//
// compute_main
//
struct modf_result_f32 {
  float fract;
  float whole;
};


void modf_bbf7f7() {
  modf_result_f32 res = {-0.5f, -1.0f};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_bbf7f7();
}

//
// vertex_main
//
struct modf_result_f32 {
  float fract;
  float whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_bbf7f7() {
  modf_result_f32 res = {-0.5f, -1.0f};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  modf_bbf7f7();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

