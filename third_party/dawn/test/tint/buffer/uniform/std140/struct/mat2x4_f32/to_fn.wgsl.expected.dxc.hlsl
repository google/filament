struct S {
  int before;
  float2x4 m;
  int after;
};

cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};

void a(S a_1[4]) {
}

void b(S s) {
}

void c(float2x4 m) {
}

void d(float4 v) {
}

void e(float f_1) {
}

float2x4 u_load_3(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]));
}

S u_load_1(uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 64u)) / 4;
  S tint_symbol = {asint(u[scalar_offset_2 / 4][scalar_offset_2 % 4]), u_load_3((offset + 16u)), asint(u[scalar_offset_3 / 4][scalar_offset_3 % 4])};
  return tint_symbol;
}

typedef S u_load_ret[4];
u_load_ret u_load(uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 128u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  a(u_load(0u));
  b(u_load_1(256u));
  c(u_load_3(272u));
  d(asfloat(u[2]).ywxz);
  e(asfloat(u[2]).ywxz.x);
  return;
}
