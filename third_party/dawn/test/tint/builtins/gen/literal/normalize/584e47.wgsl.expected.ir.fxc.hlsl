//
// fragment_main
//

void normalize_584e47() {
  float2 res = (0.70710676908493041992f).xx;
}

void fragment_main() {
  normalize_584e47();
}

//
// compute_main
//

void normalize_584e47() {
  float2 res = (0.70710676908493041992f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  normalize_584e47();
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


void normalize_584e47() {
  float2 res = (0.70710676908493041992f).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  normalize_584e47();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

