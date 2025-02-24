//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 acosh_e38f5c() {
  float3 arg_0 = (1.54308068752288818359f).xxx;
  float3 v = arg_0;
  float3 res = log((v + sqrt(((v * v) - (1.0f).xxx))));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(acosh_e38f5c()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 acosh_e38f5c() {
  float3 arg_0 = (1.54308068752288818359f).xxx;
  float3 v = arg_0;
  float3 res = log((v + sqrt(((v * v) - (1.0f).xxx))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(acosh_e38f5c()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float3 acosh_e38f5c() {
  float3 arg_0 = (1.54308068752288818359f).xxx;
  float3 v = arg_0;
  float3 res = log((v + sqrt(((v * v) - (1.0f).xxx))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = acosh_e38f5c();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

