SKIP: INVALID

uint tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asuint(uint((r.x & 0xffff) | ((r.y & 0xffff) << 16)));
}

RWByteAddressBuffer prevent_dce : register(u0);

uint bitcast_a58b50() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  uint res = tint_bitcast_from_f16(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(bitcast_a58b50()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(bitcast_a58b50()));
  return;
}

struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation uint prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = bitcast_a58b50();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(1,35-43): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(2,29-31): error X3004: undeclared identifier 'src'
<scrubbed_path>(2,22-32): error X3014: incorrect number of arguments to numeric-type constructor


tint executable returned error: exit status 1
