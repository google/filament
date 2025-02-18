struct S {
  float a;
};


ByteAddressBuffer b0 : register(t0);
ByteAddressBuffer b1 : register(t0, space1);
ByteAddressBuffer b2 : register(t0, space2);
ByteAddressBuffer b3 : register(t0, space3);
ByteAddressBuffer b4 : register(t0, space4);
ByteAddressBuffer b5 : register(t0, space5);
ByteAddressBuffer b6 : register(t0, space6);
ByteAddressBuffer b7 : register(t0, space7);
cbuffer cbuffer_b8 : register(b1, space9) {
  uint4 b8[1];
};
cbuffer cbuffer_b9 : register(b1, space8) {
  uint4 b9[1];
};
cbuffer cbuffer_b10 : register(b1, space10) {
  uint4 b10[1];
};
cbuffer cbuffer_b11 : register(b1, space11) {
  uint4 b11[1];
};
cbuffer cbuffer_b12 : register(b1, space12) {
  uint4 b12[1];
};
cbuffer cbuffer_b13 : register(b1, space13) {
  uint4 b13[1];
};
cbuffer cbuffer_b14 : register(b1, space14) {
  uint4 b14[1];
};
cbuffer cbuffer_b15 : register(b1, space15) {
  uint4 b15[1];
};
Texture2D<float4> t0 : register(t1);
Texture2D<float4> t1 : register(t1, space1);
Texture2D<float4> t2 : register(t1, space2);
Texture2D<float4> t3 : register(t1, space3);
Texture2D<float4> t4 : register(t1, space4);
Texture2D<float4> t5 : register(t1, space5);
Texture2D<float4> t6 : register(t1, space6);
Texture2D<float4> t7 : register(t1, space7);
Texture2D t8 : register(t200, space8);
Texture2D t9 : register(t200, space9);
Texture2D t10 : register(t200, space10);
Texture2D t11 : register(t200, space11);
Texture2D t12 : register(t200, space12);
Texture2D t13 : register(t200, space13);
Texture2D t14 : register(t200, space14);
Texture2D t15 : register(t200, space15);
SamplerState s0 : register(s200);
SamplerState s1 : register(s200, space1);
SamplerState s2 : register(s200, space2);
SamplerState s3 : register(s200, space3);
SamplerState s4 : register(s200, space4);
SamplerState s5 : register(s200, space5);
SamplerState s6 : register(s200, space6);
SamplerState s7 : register(s200, space7);
SamplerComparisonState s8 : register(s300, space8);
SamplerComparisonState s9 : register(s300, space9);
SamplerComparisonState s10 : register(s300, space10);
SamplerComparisonState s11 : register(s300, space11);
SamplerComparisonState s12 : register(s300, space12);
SamplerComparisonState s13 : register(s300, space13);
SamplerComparisonState s14 : register(s300, space14);
SamplerComparisonState s15 : register(s300, space15);
S v(uint start_byte_offset) {
  S v_1 = {asfloat(b15[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)])};
  return v_1;
}

S v_2(uint start_byte_offset) {
  S v_3 = {asfloat(b14[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)])};
  return v_3;
}

S v_4(uint start_byte_offset) {
  S v_5 = {asfloat(b13[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)])};
  return v_5;
}

S v_6(uint start_byte_offset) {
  S v_7 = {asfloat(b12[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)])};
  return v_7;
}

S v_8(uint start_byte_offset) {
  S v_9 = {asfloat(b11[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)])};
  return v_9;
}

S v_10(uint start_byte_offset) {
  S v_11 = {asfloat(b10[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)])};
  return v_11;
}

S v_12(uint start_byte_offset) {
  S v_13 = {asfloat(b9[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)])};
  return v_13;
}

S v_14(uint start_byte_offset) {
  S v_15 = {asfloat(b8[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)])};
  return v_15;
}

S v_16(uint offset) {
  S v_17 = {asfloat(b7.Load((offset + 0u)))};
  return v_17;
}

S v_18(uint offset) {
  S v_19 = {asfloat(b6.Load((offset + 0u)))};
  return v_19;
}

S v_20(uint offset) {
  S v_21 = {asfloat(b5.Load((offset + 0u)))};
  return v_21;
}

S v_22(uint offset) {
  S v_23 = {asfloat(b4.Load((offset + 0u)))};
  return v_23;
}

S v_24(uint offset) {
  S v_25 = {asfloat(b3.Load((offset + 0u)))};
  return v_25;
}

S v_26(uint offset) {
  S v_27 = {asfloat(b2.Load((offset + 0u)))};
  return v_27;
}

S v_28(uint offset) {
  S v_29 = {asfloat(b1.Load((offset + 0u)))};
  return v_29;
}

S v_30(uint offset) {
  S v_31 = {asfloat(b0.Load((offset + 0u)))};
  return v_31;
}

void main() {
  v_30(0u);
  v_28(0u);
  v_26(0u);
  v_24(0u);
  v_22(0u);
  v_20(0u);
  v_18(0u);
  v_16(0u);
  v_14(0u);
  v_12(0u);
  v_10(0u);
  v_8(0u);
  v_6(0u);
  v_4(0u);
  v_2(0u);
  v(0u);
}

