//
// fragment_main
//

void sqrt_4ac2c5() {
  float4 res = (1.0f).xxxx;
}

void fragment_main() {
  sqrt_4ac2c5();
}

//
// compute_main
//

void sqrt_4ac2c5() {
  float4 res = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sqrt_4ac2c5();
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


void sqrt_4ac2c5() {
  float4 res = (1.0f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  sqrt_4ac2c5();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

