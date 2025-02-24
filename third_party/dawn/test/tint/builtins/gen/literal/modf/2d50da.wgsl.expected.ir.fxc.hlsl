//
// fragment_main
//
struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};


void modf_2d50da() {
  modf_result_vec2_f32 res = {(-0.5f).xx, (-1.0f).xx};
}

void fragment_main() {
  modf_2d50da();
}

//
// compute_main
//
struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};


void modf_2d50da() {
  modf_result_vec2_f32 res = {(-0.5f).xx, (-1.0f).xx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_2d50da();
}

//
// vertex_main
//
struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_2d50da() {
  modf_result_vec2_f32 res = {(-0.5f).xx, (-1.0f).xx};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  modf_2d50da();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

