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

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_5 = float16_t(t_low.x);
  float16_t v_6 = float16_t(t_high.x);
  float16_t v_7 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_5, v_6, v_7, float16_t(t_high.y));
}

matrix<float16_t, 2, 4> v_8(uint start_byte_offset) {
  uint4 v_9 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_10 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy)));
  uint4 v_11 = u[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 4>(v_10, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_11.zw) : (v_11.xy))));
}

Inner v_12(uint start_byte_offset) {
  uint v_13 = u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  float16_t v_14 = float16_t(f16tof32((v_13 >> ((((start_byte_offset % 4u) == 0u)) ? (0u) : (16u)))));
  uint4 v_15 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_16 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_15.zw) : (v_15.xy))).xyz;
  Inner v_17 = {v_14, v_16, v_8((16u + start_byte_offset))};
  return v_17;
}

S v_18(uint start_byte_offset) {
  Inner v_19 = v_12(start_byte_offset);
  S v_20 = {v_19};
  return v_20;
}

[numthreads(1, 1, 1)]
void main() {
  S x = v_18(0u);
  v_3(0u, x);
}

