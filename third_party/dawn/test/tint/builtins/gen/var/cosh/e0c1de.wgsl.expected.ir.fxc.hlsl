//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 cosh_e0c1de() {
  float4 arg_0 = (0.0f).xxxx;
  float4 res = cosh(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(cosh_e0c1de()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 cosh_e0c1de() {
  float4 arg_0 = (0.0f).xxxx;
  float4 res = cosh(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(cosh_e0c1de()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float4 cosh_e0c1de() {
  float4 arg_0 = (0.0f).xxxx;
  float4 res = cosh(arg_0);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = cosh_e0c1de();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

