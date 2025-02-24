//
// fragment_main
//

void ceil_bb2ca2() {
  float2 res = (2.0f).xx;
}

void fragment_main() {
  ceil_bb2ca2();
}

//
// compute_main
//

void ceil_bb2ca2() {
  float2 res = (2.0f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ceil_bb2ca2();
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


void ceil_bb2ca2() {
  float2 res = (2.0f).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  ceil_bb2ca2();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

