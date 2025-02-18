//
// fragment_main
//

void floor_dcd5a2() {
  float res = 1.0f;
}

void fragment_main() {
  floor_dcd5a2();
}

//
// compute_main
//

void floor_dcd5a2() {
  float res = 1.0f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  floor_dcd5a2();
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


void floor_dcd5a2() {
  float res = 1.0f;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  floor_dcd5a2();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

