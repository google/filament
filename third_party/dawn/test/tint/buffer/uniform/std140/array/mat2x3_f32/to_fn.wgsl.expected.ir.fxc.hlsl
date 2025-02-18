
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
float a(float2x3 a_1[4]) {
  return a_1[0u][0u].x;
}

float b(float2x3 m) {
  return m[0u].x;
}

float c(float3 v) {
  return v.x;
}

float d(float f_1) {
  return f_1;
}

float2x3 v_1(uint start_byte_offset) {
  return float2x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz));
}

typedef float2x3 ary_ret[4];
ary_ret v_2(uint start_byte_offset) {
  float2x3 a_2[4] = (float2x3[4])0;
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      a_2[v_4] = v_1((start_byte_offset + (v_4 * 32u)));
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  float2x3 v_5[4] = a_2;
  return v_5;
}

[numthreads(1, 1, 1)]
void f() {
  float2x3 v_6[4] = v_2(0u);
  float v_7 = a(v_6);
  float v_8 = (v_7 + b(v_1(32u)));
  float v_9 = (v_8 + c(asfloat(u[2u].xyz).zxy));
  s.Store(0u, asuint((v_9 + d(asfloat(u[2u].xyz).zxy.x))));
}

