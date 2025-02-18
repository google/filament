//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 firstLeadingBit_35053e() {
  int3 res = (int(0)).xxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(firstLeadingBit_35053e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 firstLeadingBit_35053e() {
  int3 res = (int(0)).xxx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(firstLeadingBit_35053e()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int3 firstLeadingBit_35053e() {
  int3 res = (int(0)).xxx;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = firstLeadingBit_35053e();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

