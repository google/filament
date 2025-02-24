//
// fragment_main
//

void log2_a52bbb() {
  float4 res = (0.0f).xxxx;
}

void fragment_main() {
  log2_a52bbb();
}

//
// compute_main
//

void log2_a52bbb() {
  float4 res = (0.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log2_a52bbb();
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


void log2_a52bbb() {
  float4 res = (0.0f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  log2_a52bbb();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

