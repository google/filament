//
// fragment_main
//
struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};


void modf_5ea256() {
  modf_result_vec3_f32 res = {(-0.5f).xxx, (-1.0f).xxx};
}

void fragment_main() {
  modf_5ea256();
}

//
// compute_main
//
struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};


void modf_5ea256() {
  modf_result_vec3_f32 res = {(-0.5f).xxx, (-1.0f).xxx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_5ea256();
}

//
// vertex_main
//
struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_5ea256() {
  modf_result_vec3_f32 res = {(-0.5f).xxx, (-1.0f).xxx};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  modf_5ea256();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

