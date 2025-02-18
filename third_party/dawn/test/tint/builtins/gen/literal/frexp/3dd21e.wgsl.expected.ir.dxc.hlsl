//
// fragment_main
//
struct frexp_result_vec4_f16 {
  vector<float16_t, 4> fract;
  int4 exp;
};


void frexp_3dd21e() {
  frexp_result_vec4_f16 res = {(float16_t(0.5h)).xxxx, (int(1)).xxxx};
}

void fragment_main() {
  frexp_3dd21e();
}

//
// compute_main
//
struct frexp_result_vec4_f16 {
  vector<float16_t, 4> fract;
  int4 exp;
};


void frexp_3dd21e() {
  frexp_result_vec4_f16 res = {(float16_t(0.5h)).xxxx, (int(1)).xxxx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_3dd21e();
}

//
// vertex_main
//
struct frexp_result_vec4_f16 {
  vector<float16_t, 4> fract;
  int4 exp;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void frexp_3dd21e() {
  frexp_result_vec4_f16 res = {(float16_t(0.5h)).xxxx, (int(1)).xxxx};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  frexp_3dd21e();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

