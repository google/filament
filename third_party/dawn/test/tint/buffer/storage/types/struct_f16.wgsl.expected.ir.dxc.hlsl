struct Inner {
  float16_t scalar_f16;
  vector<float16_t, 3> vec3_f16;
  matrix<float16_t, 2, 4> mat2x4_f16;
};

struct S {
  Inner inner;
};


ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, matrix<float16_t, 2, 4> obj) {
  v_1.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  v_1.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

void v_3(uint offset, Inner obj) {
  v_1.Store<float16_t>((offset + 0u), obj.scalar_f16);
  v_1.Store<vector<float16_t, 3> >((offset + 8u), obj.vec3_f16);
  v_2((offset + 16u), obj.mat2x4_f16);
}

void v_4(uint offset, S obj) {
  Inner v_5 = obj.inner;
  v_3((offset + 0u), v_5);
}

matrix<float16_t, 2, 4> v_6(uint offset) {
  return matrix<float16_t, 2, 4>(v.Load<vector<float16_t, 4> >((offset + 0u)), v.Load<vector<float16_t, 4> >((offset + 8u)));
}

Inner v_7(uint offset) {
  float16_t v_8 = v.Load<float16_t>((offset + 0u));
  vector<float16_t, 3> v_9 = v.Load<vector<float16_t, 3> >((offset + 8u));
  Inner v_10 = {v_8, v_9, v_6((offset + 16u))};
  return v_10;
}

S v_11(uint offset) {
  Inner v_12 = v_7((offset + 0u));
  S v_13 = {v_12};
  return v_13;
}

[numthreads(1, 1, 1)]
void main() {
  S t = v_11(0u);
  v_4(0u, t);
}

