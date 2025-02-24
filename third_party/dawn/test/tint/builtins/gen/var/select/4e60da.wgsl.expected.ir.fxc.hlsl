//
// fragment_main
//

void select_4e60da() {
  bool arg_2 = true;
  float2 res = ((arg_2) ? ((1.0f).xx) : ((1.0f).xx));
}

void fragment_main() {
  select_4e60da();
}

//
// compute_main
//

void select_4e60da() {
  bool arg_2 = true;
  float2 res = ((arg_2) ? ((1.0f).xx) : ((1.0f).xx));
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_4e60da();
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


void select_4e60da() {
  bool arg_2 = true;
  float2 res = ((arg_2) ? ((1.0f).xx) : ((1.0f).xx));
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  select_4e60da();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

