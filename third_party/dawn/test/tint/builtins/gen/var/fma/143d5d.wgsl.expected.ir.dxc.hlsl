//
// fragment_main
//

void fma_143d5d() {
  float4 res = (2.0f).xxxx;
}

void fragment_main() {
  fma_143d5d();
}

//
// compute_main
//

void fma_143d5d() {
  float4 res = (2.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fma_143d5d();
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


void fma_143d5d() {
  float4 res = (2.0f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  fma_143d5d();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

