
cbuffer cbuffer_S : register(b0) {
  uint4 S[4];
};
typedef int4 ary_ret[4];
ary_ret v(uint start_byte_offset) {
  int4 a[4] = (int4[4])0;
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      a[v_2] = asint(S[((start_byte_offset + (v_2 * 16u)) / 16u)]);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  int4 v_3[4] = a;
  return v_3;
}

typedef int4 ary_ret_1[4];
ary_ret_1 func() {
  int4 v_4[4] = v(0u);
  return v_4;
}

[numthreads(1, 1, 1)]
void main() {
  int4 r[4] = func();
}

