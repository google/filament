//
// fragment_main
//

void select_e381c3() {
  bool arg_2 = true;
  int4 res = ((arg_2) ? ((int(1)).xxxx) : ((int(1)).xxxx));
}

void fragment_main() {
  select_e381c3();
}

//
// compute_main
//

void select_e381c3() {
  bool arg_2 = true;
  int4 res = ((arg_2) ? ((int(1)).xxxx) : ((int(1)).xxxx));
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_e381c3();
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


void select_e381c3() {
  bool arg_2 = true;
  int4 res = ((arg_2) ? ((int(1)).xxxx) : ((int(1)).xxxx));
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  select_e381c3();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

