//
// fragment_main
//
struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};


void modf_4bfced() {
  modf_result_vec4_f32 res = {(-0.5f).xxxx, (-1.0f).xxxx};
}

void fragment_main() {
  modf_4bfced();
}

//
// compute_main
//
struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};


void modf_4bfced() {
  modf_result_vec4_f32 res = {(-0.5f).xxxx, (-1.0f).xxxx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_4bfced();
}

//
// vertex_main
//
struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_4bfced() {
  modf_result_vec4_f32 res = {(-0.5f).xxxx, (-1.0f).xxxx};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  modf_4bfced();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

