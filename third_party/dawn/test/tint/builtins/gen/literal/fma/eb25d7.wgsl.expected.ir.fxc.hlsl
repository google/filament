//
// fragment_main
//

void fma_eb25d7() {
  float3 res = (2.0f).xxx;
}

void fragment_main() {
  fma_eb25d7();
}

//
// compute_main
//

void fma_eb25d7() {
  float3 res = (2.0f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fma_eb25d7();
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


void fma_eb25d7() {
  float3 res = (2.0f).xxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  fma_eb25d7();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

