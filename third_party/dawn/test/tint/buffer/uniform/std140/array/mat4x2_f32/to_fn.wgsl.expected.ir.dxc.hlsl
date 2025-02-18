
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
float a(float4x2 a_1[4]) {
  return a_1[0u][0u].x;
}

float b(float4x2 m) {
  return m[0u].x;
}

float c(float2 v) {
  return v.x;
}

float d(float f_1) {
  return f_1;
}

float4x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint4 v_4 = u[((8u + start_byte_offset) / 16u)];
  float2 v_5 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint4 v_6 = u[((16u + start_byte_offset) / 16u)];
  float2 v_7 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy)));
  uint4 v_8 = u[((24u + start_byte_offset) / 16u)];
  return float4x2(v_3, v_5, v_7, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy))));
}

typedef float4x2 ary_ret[4];
ary_ret v_9(uint start_byte_offset) {
  float4x2 a_2[4] = (float4x2[4])0;
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      a_2[v_11] = v_1((start_byte_offset + (v_11 * 32u)));
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
  float4x2 v_12[4] = a_2;
  return v_12;
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 v_13[4] = v_9(0u);
  float v_14 = a(v_13);
  float v_15 = (v_14 + b(v_1(32u)));
  float v_16 = (v_15 + c(asfloat(u[2u].xy).yx));
  s.Store(0u, asuint((v_16 + d(asfloat(u[2u].xy).yx.x))));
}

