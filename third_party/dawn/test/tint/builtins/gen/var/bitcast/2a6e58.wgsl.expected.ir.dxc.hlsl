//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asfloat(uint2(((r.x & 65535u) | ((r.y & 65535u) << 16u)), ((r.z & 65535u) | ((r.w & 65535u) << 16u))));
}

float2 bitcast_2a6e58() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  float2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(bitcast_2a6e58()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asfloat(uint2(((r.x & 65535u) | ((r.y & 65535u) << 16u)), ((r.z & 65535u) | ((r.w & 65535u) << 16u))));
}

float2 bitcast_2a6e58() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  float2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(bitcast_2a6e58()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asfloat(uint2(((r.x & 65535u) | ((r.y & 65535u) << 16u)), ((r.z & 65535u) | ((r.w & 65535u) << 16u))));
}

float2 bitcast_2a6e58() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  float2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = bitcast_2a6e58();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

