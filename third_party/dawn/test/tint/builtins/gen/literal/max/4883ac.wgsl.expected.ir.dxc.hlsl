//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 max_4883ac() {
  float3 res = (1.0f).xxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(max_4883ac()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 max_4883ac() {
  float3 res = (1.0f).xxx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(max_4883ac()));
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


float3 max_4883ac() {
  float3 res = (1.0f).xxx;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = max_4883ac();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

