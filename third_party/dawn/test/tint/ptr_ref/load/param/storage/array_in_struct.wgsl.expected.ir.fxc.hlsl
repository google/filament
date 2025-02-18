
ByteAddressBuffer S : register(t0);
typedef int ary_ret[4];
ary_ret v(uint offset) {
  int a[4] = (int[4])0;
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      a[v_2] = asint(S.Load((offset + (v_2 * 4u))));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  int v_3[4] = a;
  return v_3;
}

typedef int ary_ret_1[4];
ary_ret_1 func() {
  int v_4[4] = v(0u);
  return v_4;
}

[numthreads(1, 1, 1)]
void main() {
  int r[4] = func();
}

