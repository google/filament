//
// fragment_main
//

void tan_a0966f() {
  float4 res = (1.55740773677825927734f).xxxx;
}

void fragment_main() {
  tan_a0966f();
}

//
// compute_main
//

void tan_a0966f() {
  float4 res = (1.55740773677825927734f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_a0966f();
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


void tan_a0966f() {
  float4 res = (1.55740773677825927734f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  tan_a0966f();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

