
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, float obj[4]) {
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      v_1.Store((offset + (v_4 * 4u)), asuint(obj[v_4]));
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
}

typedef float ary_ret[4];
ary_ret v_5(uint offset) {
  float a[4] = (float[4])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      a[v_7] = asfloat(v.Load((offset + (v_7 * 4u))));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  float v_8[4] = a;
  return v_8;
}

[numthreads(1, 1, 1)]
void main() {
  float v_9[4] = v_5(0u);
  v_2(0u, v_9);
}

