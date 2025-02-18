//
// fragment_main
//

void tanh_313aa1() {
  float res = 0.76159417629241943359f;
}

void fragment_main() {
  tanh_313aa1();
}

//
// compute_main
//

void tanh_313aa1() {
  float res = 0.76159417629241943359f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_313aa1();
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void tanh_313aa1() {
  float res = 0.76159417629241943359f;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  tanh_313aa1();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

