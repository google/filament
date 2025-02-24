//
// fragment_main
//

void atan2_c19683() {
  float2 res = (0.78539818525314331055f).xx;
}

void fragment_main() {
  atan2_c19683();
}

//
// compute_main
//

void atan2_c19683() {
  float2 res = (0.78539818525314331055f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_c19683();
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


void atan2_c19683() {
  float2 res = (0.78539818525314331055f).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  atan2_c19683();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

