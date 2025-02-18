//
// fragment_main
//

void pow_749c42() {
  float res = 1.0f;
}

void fragment_main() {
  pow_749c42();
}

//
// compute_main
//

void pow_749c42() {
  float res = 1.0f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pow_749c42();
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


void pow_749c42() {
  float res = 1.0f;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  pow_749c42();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

