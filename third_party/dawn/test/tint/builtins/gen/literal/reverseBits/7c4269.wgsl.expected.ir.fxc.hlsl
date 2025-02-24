//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int reverseBits_7c4269() {
  int res = int(-2147483648);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(reverseBits_7c4269()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int reverseBits_7c4269() {
  int res = int(-2147483648);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(reverseBits_7c4269()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int reverseBits_7c4269() {
  int res = int(-2147483648);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = reverseBits_7c4269();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

