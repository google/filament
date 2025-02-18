//
// fragment_main
//
struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};


void frexp_eb2421() {
  frexp_result_vec2_f32 res = {(0.5f).xx, (int(1)).xx};
}

void fragment_main() {
  frexp_eb2421();
}

//
// compute_main
//
struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};


void frexp_eb2421() {
  frexp_result_vec2_f32 res = {(0.5f).xx, (int(1)).xx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_eb2421();
}

//
// vertex_main
//
struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void frexp_eb2421() {
  frexp_result_vec2_f32 res = {(0.5f).xx, (int(1)).xx};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  frexp_eb2421();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

