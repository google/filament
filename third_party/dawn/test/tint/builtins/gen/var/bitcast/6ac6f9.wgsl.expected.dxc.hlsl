//
// fragment_main
//
int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asint(uint((r.x & 0xffff) | ((r.y & 0xffff) << 16)));
}

RWByteAddressBuffer prevent_dce : register(u0);

int bitcast_6ac6f9() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  int res = tint_bitcast_from_f16(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(bitcast_6ac6f9()));
  return;
}
//
// compute_main
//
int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asint(uint((r.x & 0xffff) | ((r.y & 0xffff) << 16)));
}

RWByteAddressBuffer prevent_dce : register(u0);

int bitcast_6ac6f9() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  int res = tint_bitcast_from_f16(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(bitcast_6ac6f9()));
  return;
}
//
// vertex_main
//
int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asint(uint((r.x & 0xffff) | ((r.y & 0xffff) << 16)));
}

int bitcast_6ac6f9() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  int res = tint_bitcast_from_f16(arg_0);
  return res;
}

struct VertexOutput {
  float4 pos;
  int prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation int prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = bitcast_6ac6f9();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
