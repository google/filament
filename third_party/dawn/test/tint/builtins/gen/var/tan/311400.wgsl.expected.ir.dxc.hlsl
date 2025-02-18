//
// fragment_main
//

void tan_311400() {
  float res = 1.55740773677825927734f;
}

void fragment_main() {
  tan_311400();
}

//
// compute_main
//

void tan_311400() {
  float res = 1.55740773677825927734f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_311400();
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


void tan_311400() {
  float res = 1.55740773677825927734f;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  tan_311400();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

