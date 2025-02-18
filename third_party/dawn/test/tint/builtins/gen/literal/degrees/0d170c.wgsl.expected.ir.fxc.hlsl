//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 degrees_0d170c() {
  float4 res = (57.2957763671875f).xxxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(degrees_0d170c()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 degrees_0d170c() {
  float4 res = (57.2957763671875f).xxxx;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(degrees_0d170c()));
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


float4 degrees_0d170c() {
  float4 res = (57.2957763671875f).xxxx;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = degrees_0d170c();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

