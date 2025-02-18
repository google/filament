//
// fragment_main
//

void clamp_d396af() {
  int4 res = (int(1)).xxxx;
}

void fragment_main() {
  clamp_d396af();
}

//
// compute_main
//

void clamp_d396af() {
  int4 res = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_d396af();
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


void clamp_d396af() {
  int4 res = (int(1)).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  clamp_d396af();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

