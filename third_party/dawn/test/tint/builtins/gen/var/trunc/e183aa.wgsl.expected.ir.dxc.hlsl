//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 trunc_e183aa() {
  float4 arg_0 = (1.5f).xxxx;
  float4 v = arg_0;
  float4 res = (((v < (0.0f).xxxx)) ? (ceil(v)) : (floor(v)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(trunc_e183aa()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 trunc_e183aa() {
  float4 arg_0 = (1.5f).xxxx;
  float4 v = arg_0;
  float4 res = (((v < (0.0f).xxxx)) ? (ceil(v)) : (floor(v)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(trunc_e183aa()));
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


float4 trunc_e183aa() {
  float4 arg_0 = (1.5f).xxxx;
  float4 v = arg_0;
  float4 res = (((v < (0.0f).xxxx)) ? (ceil(v)) : (floor(v)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = trunc_e183aa();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

