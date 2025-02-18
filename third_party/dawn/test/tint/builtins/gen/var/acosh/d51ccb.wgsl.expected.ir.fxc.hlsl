//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 acosh_d51ccb() {
  float4 arg_0 = (1.54308068752288818359f).xxxx;
  float4 v = arg_0;
  float4 res = log((v + sqrt(((v * v) - (1.0f).xxxx))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(acosh_d51ccb()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 acosh_d51ccb() {
  float4 arg_0 = (1.54308068752288818359f).xxxx;
  float4 v = arg_0;
  float4 res = log((v + sqrt(((v * v) - (1.0f).xxxx))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(acosh_d51ccb()));
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


float4 acosh_d51ccb() {
  float4 arg_0 = (1.54308068752288818359f).xxxx;
  float4 v = arg_0;
  float4 res = log((v + sqrt(((v * v) - (1.0f).xxxx))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = acosh_d51ccb();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

