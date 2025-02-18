//
// fragment_main
//

void fract_7e3f2d() {
  float4 res = (0.25f).xxxx;
}

void fragment_main() {
  fract_7e3f2d();
}

//
// compute_main
//

void fract_7e3f2d() {
  float4 res = (0.25f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_7e3f2d();
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


void fract_7e3f2d() {
  float4 res = (0.25f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  fract_7e3f2d();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

