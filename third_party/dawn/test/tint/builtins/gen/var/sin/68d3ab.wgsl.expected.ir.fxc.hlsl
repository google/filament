//
// fragment_main
//

void sin_68d3ab() {
  float2 res = (1.0f).xx;
}

void fragment_main() {
  sin_68d3ab();
}

//
// compute_main
//

void sin_68d3ab() {
  float2 res = (1.0f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sin_68d3ab();
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


void sin_68d3ab() {
  float2 res = (1.0f).xx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  sin_68d3ab();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

