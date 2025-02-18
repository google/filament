//
// fragment_main
//

void select_17441a() {
  bool arg_2 = true;
  float4 res = ((arg_2) ? ((1.0f).xxxx) : ((1.0f).xxxx));
}

void fragment_main() {
  select_17441a();
}

//
// compute_main
//

void select_17441a() {
  bool arg_2 = true;
  float4 res = ((arg_2) ? ((1.0f).xxxx) : ((1.0f).xxxx));
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_17441a();
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


void select_17441a() {
  bool arg_2 = true;
  float4 res = ((arg_2) ? ((1.0f).xxxx) : ((1.0f).xxxx));
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  select_17441a();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

