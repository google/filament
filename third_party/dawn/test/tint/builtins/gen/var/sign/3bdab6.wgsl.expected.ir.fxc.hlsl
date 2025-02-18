//
// fragment_main
//

void sign_3bdab6() {
  int4 res = (int(1)).xxxx;
}

void fragment_main() {
  sign_3bdab6();
}

//
// compute_main
//

void sign_3bdab6() {
  int4 res = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sign_3bdab6();
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


void sign_3bdab6() {
  int4 res = (int(1)).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  sign_3bdab6();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

