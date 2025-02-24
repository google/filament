//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float determinant_a0a87c() {
  float4x4 arg_0 = float4x4((1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx);
  float res = determinant(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(determinant_a0a87c()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float determinant_a0a87c() {
  float4x4 arg_0 = float4x4((1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx);
  float res = determinant(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(determinant_a0a87c()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float determinant_a0a87c() {
  float4x4 arg_0 = float4x4((1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx, (1.0f).xxxx);
  float res = determinant(arg_0);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = determinant_a0a87c();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

