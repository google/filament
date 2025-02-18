//
// fragment_main
//

void radians_44a9f8() {
  float2 res = (0.01745329238474369049f).xx;
}

void fragment_main() {
  radians_44a9f8();
}

//
// compute_main
//

void radians_44a9f8() {
  float2 res = (0.01745329238474369049f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  radians_44a9f8();
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


void radians_44a9f8() {
  float2 res = (0.01745329238474369049f).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  radians_44a9f8();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

