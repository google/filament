//
// fragment_main
//
uint2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asuint(uint2((r.x & 0xffff) | ((r.y & 0xffff) << 16), (r.z & 0xffff) | ((r.w & 0xffff) << 16)));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 bitcast_81c5f5() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  uint2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(bitcast_81c5f5()));
  return;
}
//
// compute_main
//
uint2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asuint(uint2((r.x & 0xffff) | ((r.y & 0xffff) << 16), (r.z & 0xffff) | ((r.w & 0xffff) << 16)));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint2 bitcast_81c5f5() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  uint2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(bitcast_81c5f5()));
  return;
}
//
// vertex_main
//
uint2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asuint(uint2((r.x & 0xffff) | ((r.y & 0xffff) << 16), (r.z & 0xffff) | ((r.w & 0xffff) << 16)));
}

uint2 bitcast_81c5f5() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  uint2 res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation uint2 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = bitcast_81c5f5();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
