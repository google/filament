//
// fragment_main
//

void asinh_cf8603() {
  float4 res = (0.88137358427047729492f).xxxx;
}

void fragment_main() {
  asinh_cf8603();
}

//
// compute_main
//

void asinh_cf8603() {
  float4 res = (0.88137358427047729492f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asinh_cf8603();
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


void asinh_cf8603() {
  float4 res = (0.88137358427047729492f).xxxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  asinh_cf8603();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

