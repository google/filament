//
// fragment_main
//
struct frexp_result_vec3_f32 {
  float3 fract;
  int3 exp;
};


void frexp_979800() {
  frexp_result_vec3_f32 res = {(0.5f).xxx, (int(1)).xxx};
}

void fragment_main() {
  frexp_979800();
}

//
// compute_main
//
struct frexp_result_vec3_f32 {
  float3 fract;
  int3 exp;
};


void frexp_979800() {
  frexp_result_vec3_f32 res = {(0.5f).xxx, (int(1)).xxx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_979800();
}

//
// vertex_main
//
struct frexp_result_vec3_f32 {
  float3 fract;
  int3 exp;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void frexp_979800() {
  frexp_result_vec3_f32 res = {(0.5f).xxx, (int(1)).xxx};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  frexp_979800();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

