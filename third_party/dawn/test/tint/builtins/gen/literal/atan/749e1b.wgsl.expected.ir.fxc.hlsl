//
// fragment_main
//

void atan_749e1b() {
  float3 res = (0.78539818525314331055f).xxx;
}

void fragment_main() {
  atan_749e1b();
}

//
// compute_main
//

void atan_749e1b() {
  float3 res = (0.78539818525314331055f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan_749e1b();
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


void atan_749e1b() {
  float3 res = (0.78539818525314331055f).xxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  atan_749e1b();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

