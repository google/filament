//
// fragment_main
//

void sinh_a3da7c() {
  float4 res = (1.17520117759704589844f).xxxx;
}

void fragment_main() {
  sinh_a3da7c();
}

//
// compute_main
//

void sinh_a3da7c() {
  float4 res = (1.17520117759704589844f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sinh_a3da7c();
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


void sinh_a3da7c() {
  float4 res = (1.17520117759704589844f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  sinh_a3da7c();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

