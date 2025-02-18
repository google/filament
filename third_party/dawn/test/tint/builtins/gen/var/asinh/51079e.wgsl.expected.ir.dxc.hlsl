//
// fragment_main
//

void asinh_51079e() {
  float3 res = (0.88137358427047729492f).xxx;
}

void fragment_main() {
  asinh_51079e();
}

//
// compute_main
//

void asinh_51079e() {
  float3 res = (0.88137358427047729492f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asinh_51079e();
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


void asinh_51079e() {
  float3 res = (0.88137358427047729492f).xxx;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  asinh_51079e();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

