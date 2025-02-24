//
// fragment_main
//

void smoothstep_a80fff() {
  float res = 0.5f;
}

void fragment_main() {
  smoothstep_a80fff();
}

//
// compute_main
//

void smoothstep_a80fff() {
  float res = 0.5f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothstep_a80fff();
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


void smoothstep_a80fff() {
  float res = 0.5f;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  smoothstep_a80fff();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

