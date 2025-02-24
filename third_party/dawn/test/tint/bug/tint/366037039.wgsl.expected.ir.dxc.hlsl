struct S {
  uint3 a;
  uint b;
  uint3 c[4];
};


cbuffer cbuffer_ubuffer : register(b0) {
  uint4 ubuffer[5];
};
RWByteAddressBuffer sbuffer : register(u1);
groupshared S wbuffer;
void v(uint offset, uint3 obj[4]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      sbuffer.Store3((offset + (v_2 * 16u)), obj[v_2]);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}

void v_3(uint offset, S obj) {
  sbuffer.Store3((offset + 0u), obj.a);
  sbuffer.Store((offset + 12u), obj.b);
  uint3 v_4[4] = obj.c;
  v((offset + 16u), v_4);
}

typedef uint3 ary_ret[4];
ary_ret v_5(uint offset) {
  uint3 a[4] = (uint3[4])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      a[v_7] = sbuffer.Load3((offset + (v_7 * 16u)));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  uint3 v_8[4] = a;
  return v_8;
}

S v_9(uint offset) {
  uint3 v_10 = sbuffer.Load3((offset + 0u));
  uint v_11 = sbuffer.Load((offset + 12u));
  uint3 v_12[4] = v_5((offset + 16u));
  S v_13 = {v_10, v_11, v_12};
  return v_13;
}

typedef uint3 ary_ret_1[4];
ary_ret_1 v_14(uint start_byte_offset) {
  uint3 a[4] = (uint3[4])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      a[v_16] = ubuffer[((start_byte_offset + (v_16 * 16u)) / 16u)].xyz;
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  uint3 v_17[4] = a;
  return v_17;
}

S v_18(uint start_byte_offset) {
  uint3 v_19 = ubuffer[(start_byte_offset / 16u)].xyz;
  uint v_20 = ubuffer[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)];
  uint3 v_21[4] = v_14((16u + start_byte_offset));
  S v_22 = {v_19, v_20, v_21};
  return v_22;
}

void foo() {
  S u = v_18(0u);
  S s = v_9(0u);
  S w = v_9(0u);
  S v_23 = (S)0;
  v_3(0u, v_23);
  wbuffer = v_23;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

