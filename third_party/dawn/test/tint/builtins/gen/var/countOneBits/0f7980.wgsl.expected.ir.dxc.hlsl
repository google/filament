//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 countOneBits_0f7980() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = asint(countbits(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(countOneBits_0f7980()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 countOneBits_0f7980() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = asint(countbits(asuint(arg_0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(countOneBits_0f7980()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int4 countOneBits_0f7980() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = asint(countbits(asuint(arg_0)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = countOneBits_0f7980();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

