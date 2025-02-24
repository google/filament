//
// fragment_main
//

void tanh_c48aa6() {
  float2 res = (0.76159417629241943359f).xx;
}

void fragment_main() {
  tanh_c48aa6();
}

//
// compute_main
//

void tanh_c48aa6() {
  float2 res = (0.76159417629241943359f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_c48aa6();
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


void tanh_c48aa6() {
  float2 res = (0.76159417629241943359f).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  tanh_c48aa6();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

