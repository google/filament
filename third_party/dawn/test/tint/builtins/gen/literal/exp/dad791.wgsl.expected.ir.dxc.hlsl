//
// fragment_main
//

void exp_dad791() {
  float4 res = (2.71828174591064453125f).xxxx;
}

void fragment_main() {
  exp_dad791();
}

//
// compute_main
//

void exp_dad791() {
  float4 res = (2.71828174591064453125f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp_dad791();
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


void exp_dad791() {
  float4 res = (2.71828174591064453125f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  exp_dad791();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

