SKIP: INVALID


ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, float16_t obj[4]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      tint_symbol_1.Store<float16_t>((offset + (v_2 * 2u)), obj[v_2]);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}

typedef float16_t ary_ret[4];
ary_ret v_3(uint offset) {
  float16_t a[4] = (float16_t[4])0;
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = tint_symbol.Load<float16_t>((offset + (v_5 * 2u)));
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  float16_t v_6[4] = a;
  return v_6;
}

[numthreads(1, 1, 1)]
void main() {
  float16_t v_7[4] = v_3(0u);
  v(0u, v_7);
}

FXC validation failure:
<scrubbed_path>(4,21-29): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
