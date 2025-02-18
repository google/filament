//
// fragment_main
//
struct frexp_result_vec4_f16 {
  vector<float16_t, 4> fract;
  int4 exp;
};


void frexp_3dd21e() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  vector<float16_t, 4> v = arg_0;
  vector<float16_t, 4> v_1 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> v_2 = frexp(v, v_1);
  vector<float16_t, 4> v_3 = (vector<float16_t, 4>(sign(v)) * v_2);
  frexp_result_vec4_f16 res = {v_3, int4(v_1)};
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
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  vector<float16_t, 4> v = arg_0;
  vector<float16_t, 4> v_1 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> v_2 = frexp(v, v_1);
  vector<float16_t, 4> v_3 = (vector<float16_t, 4>(sign(v)) * v_2);
  frexp_result_vec4_f16 res = {v_3, int4(v_1)};
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
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  vector<float16_t, 4> v = arg_0;
  vector<float16_t, 4> v_1 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> v_2 = frexp(v, v_1);
  vector<float16_t, 4> v_3 = (vector<float16_t, 4>(sign(v)) * v_2);
  frexp_result_vec4_f16 res = {v_3, int4(v_1)};
}

VertexOutput vertex_main_inner() {
  VertexOutput v_4 = (VertexOutput)0;
  v_4.pos = (0.0f).xxxx;
  frexp_3dd21e();
  VertexOutput v_5 = v_4;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.pos};
  return v_7;
}

