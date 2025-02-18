//
// fragment_main
//

void transpose_66fce8() {
  float3x3 res = float3x3((1.0f).xxx, (1.0f).xxx, (1.0f).xxx);
}

void fragment_main() {
  transpose_66fce8();
}

//
// compute_main
//

void transpose_66fce8() {
  float3x3 res = float3x3((1.0f).xxx, (1.0f).xxx, (1.0f).xxx);
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_66fce8();
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


void transpose_66fce8() {
  float3x3 res = float3x3((1.0f).xxx, (1.0f).xxx, (1.0f).xxx);
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  transpose_66fce8();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

