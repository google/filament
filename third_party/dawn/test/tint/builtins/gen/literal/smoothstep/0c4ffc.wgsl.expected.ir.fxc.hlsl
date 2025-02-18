//
// fragment_main
//

void smoothstep_0c4ffc() {
  float4 res = (0.5f).xxxx;
}

void fragment_main() {
  smoothstep_0c4ffc();
}

//
// compute_main
//

void smoothstep_0c4ffc() {
  float4 res = (0.5f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothstep_0c4ffc();
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


void smoothstep_0c4ffc() {
  float4 res = (0.5f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  smoothstep_0c4ffc();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

