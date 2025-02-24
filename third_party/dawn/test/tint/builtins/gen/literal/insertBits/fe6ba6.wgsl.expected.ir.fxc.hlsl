//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 insertBits_fe6ba6() {
  int2 res = (int(3)).xx;
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(insertBits_fe6ba6()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 insertBits_fe6ba6() {
  int2 res = (int(3)).xx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(insertBits_fe6ba6()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int2 insertBits_fe6ba6() {
  int2 res = (int(3)).xx;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = insertBits_fe6ba6();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

