//
// fragment_main
//
struct frexp_result_f32 {
  float fract;
  int exp;
};


void frexp_bee870() {
  frexp_result_f32 res = {0.5f, int(1)};
}

void fragment_main() {
  frexp_bee870();
}

//
// compute_main
//
struct frexp_result_f32 {
  float fract;
  int exp;
};


void frexp_bee870() {
  frexp_result_f32 res = {0.5f, int(1)};
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_bee870();
}

//
// vertex_main
//
struct frexp_result_f32 {
  float fract;
  int exp;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void frexp_bee870() {
  frexp_result_f32 res = {0.5f, int(1)};
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  frexp_bee870();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

