//
// fragment_main
//

void reflect_bba2d0() {
  float2 res = (-3.0f).xx;
}

void fragment_main() {
  reflect_bba2d0();
}

//
// compute_main
//

void reflect_bba2d0() {
  float2 res = (-3.0f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reflect_bba2d0();
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


void reflect_bba2d0() {
  float2 res = (-3.0f).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  reflect_bba2d0();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

