//
// fragment_main
//

void smoothstep_66e4bd() {
  float3 res = (0.5f).xxx;
}

void fragment_main() {
  smoothstep_66e4bd();
}

//
// compute_main
//

void smoothstep_66e4bd() {
  float3 res = (0.5f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothstep_66e4bd();
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


void smoothstep_66e4bd() {
  float3 res = (0.5f).xxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  smoothstep_66e4bd();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

