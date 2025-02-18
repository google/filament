//
// fragment_main
//

void degrees_d43a49() {
  float4 res = (57.295780181884765625f).xxxx;
}

void fragment_main() {
  degrees_d43a49();
}

//
// compute_main
//

void degrees_d43a49() {
  float4 res = (57.295780181884765625f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  degrees_d43a49();
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


void degrees_d43a49() {
  float4 res = (57.295780181884765625f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  degrees_d43a49();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

