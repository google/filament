//
// fragment_main
//

void degrees_c0880c() {
  float3 res = (57.295780181884765625f).xxx;
}

void fragment_main() {
  degrees_c0880c();
}

//
// compute_main
//

void degrees_c0880c() {
  float3 res = (57.295780181884765625f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  degrees_c0880c();
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


void degrees_c0880c() {
  float3 res = (57.295780181884765625f).xxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  degrees_c0880c();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

