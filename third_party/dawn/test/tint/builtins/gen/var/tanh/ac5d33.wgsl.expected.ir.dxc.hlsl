//
// fragment_main
//

void tanh_ac5d33() {
  float4 res = (0.76159417629241943359f).xxxx;
}

void fragment_main() {
  tanh_ac5d33();
}

//
// compute_main
//

void tanh_ac5d33() {
  float4 res = (0.76159417629241943359f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_ac5d33();
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


void tanh_ac5d33() {
  float4 res = (0.76159417629241943359f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  tanh_ac5d33();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

