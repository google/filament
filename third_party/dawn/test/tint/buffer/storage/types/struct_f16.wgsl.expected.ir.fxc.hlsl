SKIP: INVALID

struct Inner {
  float16_t scalar_f16;
  vector<float16_t, 3> vec3_f16;
  matrix<float16_t, 2, 4> mat2x4_f16;
};

struct S {
  Inner inner;
};


ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, matrix<float16_t, 2, 4> obj) {
  tint_symbol_1.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  tint_symbol_1.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

void v_1(uint offset, Inner obj) {
  tint_symbol_1.Store<float16_t>((offset + 0u), obj.scalar_f16);
  tint_symbol_1.Store<vector<float16_t, 3> >((offset + 8u), obj.vec3_f16);
  v((offset + 16u), obj.mat2x4_f16);
}

void v_2(uint offset, S obj) {
  Inner v_3 = obj.inner;
  v_1((offset + 0u), v_3);
}

matrix<float16_t, 2, 4> v_4(uint offset) {
  vector<float16_t, 4> v_5 = tint_symbol.Load<vector<float16_t, 4> >((offset + 0u));
  return matrix<float16_t, 2, 4>(v_5, tint_symbol.Load<vector<float16_t, 4> >((offset + 8u)));
}

Inner v_6(uint offset) {
  float16_t v_7 = tint_symbol.Load<float16_t>((offset + 0u));
  vector<float16_t, 3> v_8 = tint_symbol.Load<vector<float16_t, 3> >((offset + 8u));
  Inner v_9 = {v_7, v_8, v_4((offset + 16u))};
  return v_9;
}

S v_10(uint offset) {
  Inner v_11 = v_6((offset + 0u));
  S v_12 = {v_11};
  return v_12;
}

[numthreads(1, 1, 1)]
void main() {
  S v_13 = v_10(0u);
  S t = v_13;
  v_2(0u, t);
}

FXC validation failure:
<scrubbed_path>(2,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
