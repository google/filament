void set_matrix_column(inout float3x3 mat, int col, float3 val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
    case 2: mat[2] = val; break;
  }
}

struct S {
  float3x3 m;
};
struct S2 {
  float3x3 m[1];
};
struct S3 {
  S s;
};
struct S4 {
  S s[1];
};

RWByteAddressBuffer buffer0 : register(u0);
RWByteAddressBuffer buffer1 : register(u1);
RWByteAddressBuffer buffer2 : register(u2);
RWByteAddressBuffer buffer3 : register(u3);
RWByteAddressBuffer buffer4 : register(u4);
RWByteAddressBuffer buffer5 : register(u5);
RWByteAddressBuffer buffer6 : register(u6);
RWByteAddressBuffer buffer7 : register(u7);

void buffer0_store(uint offset, float3x3 value) {
  buffer0.Store3((offset + 0u), asuint(value[0u]));
  buffer0.Store3((offset + 16u), asuint(value[1u]));
  buffer0.Store3((offset + 32u), asuint(value[2u]));
}

void buffer1_store_1(uint offset, float3x3 value) {
  buffer1.Store3((offset + 0u), asuint(value[0u]));
  buffer1.Store3((offset + 16u), asuint(value[1u]));
  buffer1.Store3((offset + 32u), asuint(value[2u]));
}

void buffer1_store(uint offset, S value) {
  buffer1_store_1((offset + 0u), value.m);
}

void buffer2_store_2(uint offset, float3x3 value) {
  buffer2.Store3((offset + 0u), asuint(value[0u]));
  buffer2.Store3((offset + 16u), asuint(value[1u]));
  buffer2.Store3((offset + 32u), asuint(value[2u]));
}

void buffer2_store_1(uint offset, float3x3 value[1]) {
  float3x3 array_1[1] = value;
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      buffer2_store_2((offset + (i * 48u)), array_1[i]);
    }
  }
}

void buffer2_store(uint offset, S2 value) {
  buffer2_store_1((offset + 0u), value.m);
}

void buffer3_store_2(uint offset, float3x3 value) {
  buffer3.Store3((offset + 0u), asuint(value[0u]));
  buffer3.Store3((offset + 16u), asuint(value[1u]));
  buffer3.Store3((offset + 32u), asuint(value[2u]));
}

void buffer3_store_1(uint offset, S value) {
  buffer3_store_2((offset + 0u), value.m);
}

void buffer3_store(uint offset, S3 value) {
  buffer3_store_1((offset + 0u), value.s);
}

void buffer4_store_3(uint offset, float3x3 value) {
  buffer4.Store3((offset + 0u), asuint(value[0u]));
  buffer4.Store3((offset + 16u), asuint(value[1u]));
  buffer4.Store3((offset + 32u), asuint(value[2u]));
}

void buffer4_store_2(uint offset, S value) {
  buffer4_store_3((offset + 0u), value.m);
}

void buffer4_store_1(uint offset, S value[1]) {
  S array_2[1] = value;
  {
    for(uint i_1 = 0u; (i_1 < 1u); i_1 = (i_1 + 1u)) {
      buffer4_store_2((offset + (i_1 * 48u)), array_2[i_1]);
    }
  }
}

void buffer4_store(uint offset, S4 value) {
  buffer4_store_1((offset + 0u), value.s);
}

void buffer5_store_1(uint offset, float3x3 value) {
  buffer5.Store3((offset + 0u), asuint(value[0u]));
  buffer5.Store3((offset + 16u), asuint(value[1u]));
  buffer5.Store3((offset + 32u), asuint(value[2u]));
}

void buffer5_store(uint offset, float3x3 value[1]) {
  float3x3 array_3[1] = value;
  {
    for(uint i_2 = 0u; (i_2 < 1u); i_2 = (i_2 + 1u)) {
      buffer5_store_1((offset + (i_2 * 48u)), array_3[i_2]);
    }
  }
}

void buffer6_store_2(uint offset, float3x3 value) {
  buffer6.Store3((offset + 0u), asuint(value[0u]));
  buffer6.Store3((offset + 16u), asuint(value[1u]));
  buffer6.Store3((offset + 32u), asuint(value[2u]));
}

void buffer6_store_1(uint offset, S value) {
  buffer6_store_2((offset + 0u), value.m);
}

void buffer6_store(uint offset, S value[1]) {
  S array_4[1] = value;
  {
    for(uint i_3 = 0u; (i_3 < 1u); i_3 = (i_3 + 1u)) {
      buffer6_store_1((offset + (i_3 * 48u)), array_4[i_3]);
    }
  }
}

void buffer7_store_3(uint offset, float3x3 value) {
  buffer7.Store3((offset + 0u), asuint(value[0u]));
  buffer7.Store3((offset + 16u), asuint(value[1u]));
  buffer7.Store3((offset + 32u), asuint(value[2u]));
}

void buffer7_store_2(uint offset, float3x3 value[1]) {
  float3x3 array_6[1] = value;
  {
    for(uint i_4 = 0u; (i_4 < 1u); i_4 = (i_4 + 1u)) {
      buffer7_store_3((offset + (i_4 * 48u)), array_6[i_4]);
    }
  }
}

void buffer7_store_1(uint offset, S2 value) {
  buffer7_store_2((offset + 0u), value.m);
}

void buffer7_store(uint offset, S2 value[1]) {
  S2 array_5[1] = value;
  {
    for(uint i_5 = 0u; (i_5 < 1u); i_5 = (i_5 + 1u)) {
      buffer7_store_1((offset + (i_5 * 48u)), array_5[i_5]);
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  float3x3 m = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  {
    for(uint c = 0u; (c < 3u); c = (c + 1u)) {
      set_matrix_column(m, min(c, 2u), float3(float(((c * 3u) + 1u)), float(((c * 3u) + 2u)), float(((c * 3u) + 3u))));
    }
  }
  {
    float3x3 a = m;
    buffer0_store(0u, a);
  }
  {
    S a = {m};
    buffer1_store(0u, a);
  }
  {
    float3x3 tint_symbol[1] = {m};
    S2 a = {tint_symbol};
    buffer2_store(0u, a);
  }
  {
    S tint_symbol_1 = {m};
    S3 a = {tint_symbol_1};
    buffer3_store(0u, a);
  }
  {
    S tint_symbol_2 = {m};
    S tint_symbol_3[1] = {tint_symbol_2};
    S4 a = {tint_symbol_3};
    buffer4_store(0u, a);
  }
  {
    float3x3 a[1] = {m};
    buffer5_store(0u, a);
  }
  {
    S tint_symbol_4 = {m};
    S a[1] = {tint_symbol_4};
    buffer6_store(0u, a);
  }
  {
    float3x3 tint_symbol_5[1] = {m};
    S2 tint_symbol_6 = {tint_symbol_5};
    S2 a[1] = {tint_symbol_6};
    buffer7_store(0u, a);
  }
  return;
}
