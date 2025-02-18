//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quantizeToF16_cba294() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = f16tof32(f32tof16(arg_0));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quantizeToF16_cba294()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quantizeToF16_cba294() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = f16tof32(f32tof16(arg_0));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quantizeToF16_cba294()));
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


float4 quantizeToF16_cba294() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = f16tof32(f32tof16(arg_0));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = quantizeToF16_cba294();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

