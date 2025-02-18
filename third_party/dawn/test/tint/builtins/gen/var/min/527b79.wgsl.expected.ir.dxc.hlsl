//
// fragment_main
//

void min_527b79() {
  int2 res = (int(1)).xx;
}

void fragment_main() {
  min_527b79();
}

//
// compute_main
//

void min_527b79() {
  int2 res = (int(1)).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_527b79();
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


void min_527b79() {
  int2 res = (int(1)).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  min_527b79();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

