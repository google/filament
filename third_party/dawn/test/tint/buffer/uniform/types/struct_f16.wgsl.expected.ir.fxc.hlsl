SKIP: INVALID

struct Inner {
  float16_t scalar_f16;
  vector<float16_t, 3> vec3_f16;
  matrix<float16_t, 2, 4> mat2x4_f16;
};

struct S {
  Inner inner;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
RWByteAddressBuffer s : register(u1);
void v_1(uint offset, matrix<float16_t, 2, 4> obj) {
  s.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

void v_2(uint offset, Inner obj) {
  s.Store<float16_t>((offset + 0u), obj.scalar_f16);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj.vec3_f16);
  v_1((offset + 16u), obj.mat2x4_f16);
}

void v_3(uint offset, S obj) {
  Inner v_4 = obj.inner;
  v_2((offset + 0u), v_4);
}

vector<float16_t, 4> tint_bitcast_to_f16(uint4 src) {
  uint4 v = src;
  uint4 mask = (65535u).xxxx;
  uint4 shift = (16u).xxxx;
  float4 t_low = f16tof32((v & mask));
  float4 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_5 = float16_t(t_low.x);
  float16_t v_6 = float16_t(t_high.x);
  float16_t v_7 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_5, v_6, v_7, float16_t(t_high.y));
}

matrix<float16_t, 2, 4> v_8(uint start_byte_offset) {
  vector<float16_t, 4> v_9 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]);
  return matrix<float16_t, 2, 4>(v_9, tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]));
}

Inner v_10(uint start_byte_offset) {
  uint v_11 = u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  float16_t v_12 = float16_t(f16tof32((v_11 >> ((((start_byte_offset % 4u) == 0u)) ? (0u) : (16u)))));
  vector<float16_t, 3> v_13 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]).xyz;
  Inner v_14 = {v_12, v_13, v_8((16u + start_byte_offset))};
  return v_14;
}

S v_15(uint start_byte_offset) {
  Inner v_16 = v_10(start_byte_offset);
  S v_17 = {v_16};
  return v_17;
}

[numthreads(1, 1, 1)]
void main() {
  S v_18 = v_15(0u);
  S x = v_18;
  v_3(0u, x);
}

FXC validation failure:
<scrubbed_path>(2,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
