//
// fragment_main
//
struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};


void modf_5ea256() {
  float3 arg_0 = (-1.5f).xxx;
  float3 v = (0.0f).xxx;
  modf_result_vec3_f32 res = {modf(arg_0, v), v};
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
  float3 arg_0 = (-1.5f).xxx;
  float3 v = (0.0f).xxx;
  modf_result_vec3_f32 res = {modf(arg_0, v), v};
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
  float3 arg_0 = (-1.5f).xxx;
  float3 v = (0.0f).xxx;
  modf_result_vec3_f32 res = {modf(arg_0, v), v};
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  modf_5ea256();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.pos};
  return v_4;
}

