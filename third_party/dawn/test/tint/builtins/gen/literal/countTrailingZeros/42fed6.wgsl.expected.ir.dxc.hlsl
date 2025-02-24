//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int countTrailingZeros_42fed6() {
  int res = int(0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(countTrailingZeros_42fed6()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int countTrailingZeros_42fed6() {
  int res = int(0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(countTrailingZeros_42fed6()));
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


int countTrailingZeros_42fed6() {
  int res = int(0);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = countTrailingZeros_42fed6();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

