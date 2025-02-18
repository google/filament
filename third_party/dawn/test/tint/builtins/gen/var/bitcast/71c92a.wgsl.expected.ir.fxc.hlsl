SKIP: INVALID

struct VertexOutput {
  float4 pos;
  vector<float16_t, 4> prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation vector<float16_t, 4> VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> tint_bitcast_to_f16(int2 src) {
  uint2 v = asuint(src);
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

vector<float16_t, 4> bitcast_71c92a() {
  int2 arg_0 = (int(1)).xx;
  vector<float16_t, 4> res = tint_bitcast_to_f16(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, bitcast_71c92a());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, bitcast_71c92a());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = bitcast_71c92a();
  VertexOutput v_4 = tint_symbol;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  VertexOutput v_6 = v_5;
  VertexOutput v_7 = v_5;
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_6.pos};
  return v_8;
}

FXC validation failure:
<scrubbed_path>(3,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
