//
// fragment_main
//

void atanh_7f2874() {
  float3 res = (0.54930615425109863281f).xxx;
}

void fragment_main() {
  atanh_7f2874();
}

//
// compute_main
//

void atanh_7f2874() {
  float3 res = (0.54930615425109863281f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_7f2874();
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


void atanh_7f2874() {
  float3 res = (0.54930615425109863281f).xxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  atanh_7f2874();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

