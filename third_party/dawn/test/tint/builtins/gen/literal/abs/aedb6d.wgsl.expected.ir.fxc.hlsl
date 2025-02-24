//
// fragment_main
//

void abs_aedb6d() {
  float res = 1.0f;
}

void fragment_main() {
  abs_aedb6d();
}

//
// compute_main
//

void abs_aedb6d() {
  float res = 1.0f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_aedb6d();
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


void abs_aedb6d() {
  float res = 1.0f;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  abs_aedb6d();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

