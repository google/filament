//
// fragment_main
//

void distance_f9c9ee() {
  float res = 0.0f;
}

void fragment_main() {
  distance_f9c9ee();
}

//
// compute_main
//

void distance_f9c9ee() {
  float res = 0.0f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  distance_f9c9ee();
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


void distance_f9c9ee() {
  float res = 0.0f;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  distance_f9c9ee();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

