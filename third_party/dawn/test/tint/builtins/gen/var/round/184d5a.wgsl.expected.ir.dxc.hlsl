//
// fragment_main
//

void round_184d5a() {
  float4 res = (4.0f).xxxx;
}

void fragment_main() {
  round_184d5a();
}

//
// compute_main
//

void round_184d5a() {
  float4 res = (4.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  round_184d5a();
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


void round_184d5a() {
  float4 res = (4.0f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  round_184d5a();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

