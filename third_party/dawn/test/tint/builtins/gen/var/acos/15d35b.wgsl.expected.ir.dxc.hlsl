//
// fragment_main
//

void acos_15d35b() {
  float2 res = (0.25f).xx;
}

void fragment_main() {
  acos_15d35b();
}

//
// compute_main
//

void acos_15d35b() {
  float2 res = (0.25f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_15d35b();
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


void acos_15d35b() {
  float2 res = (0.25f).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  acos_15d35b();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

