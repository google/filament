//
// fragment_main
//

void fract_ed00ca() {
  float2 res = (0.25f).xx;
}

void fragment_main() {
  fract_ed00ca();
}

//
// compute_main
//

void fract_ed00ca() {
  float2 res = (0.25f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_ed00ca();
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


void fract_ed00ca() {
  float2 res = (0.25f).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  fract_ed00ca();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

