#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct S {
  float f;
  uint u;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  uint tint_pad_7;
  uint tint_pad_8;
  uint tint_pad_9;
  uint tint_pad_10;
  uint tint_pad_11;
  uint tint_pad_12;
  uint tint_pad_13;
  uint tint_pad_14;
  uint tint_pad_15;
  uint tint_pad_16;
  uint tint_pad_17;
  uint tint_pad_18;
  uint tint_pad_19;
  uint tint_pad_20;
  uint tint_pad_21;
  uint tint_pad_22;
  uint tint_pad_23;
  uint tint_pad_24;
  uint tint_pad_25;
  uint tint_pad_26;
  uint tint_pad_27;
  uint tint_pad_28;
  uint tint_pad_29;
  vec4 v;
  uint tint_pad_30;
  uint tint_pad_31;
  uint tint_pad_32;
  uint tint_pad_33;
  float16_t x;
  uint tint_pad_34;
  uint tint_pad_35;
  uint tint_pad_36;
  uint tint_pad_37;
  uint tint_pad_38;
  uint tint_pad_39;
  uint tint_pad_40;
  f16vec3 y;
  uint tint_pad_41;
  uint tint_pad_42;
  uint tint_pad_43;
  uint tint_pad_44;
  uint tint_pad_45;
  uint tint_pad_46;
  uint tint_pad_47;
  uint tint_pad_48;
  uint tint_pad_49;
  uint tint_pad_50;
  uint tint_pad_51;
  uint tint_pad_52;
  uint tint_pad_53;
  uint tint_pad_54;
};

layout(binding = 0, std430)
buffer f_output_block_ssbo {
  S inner;
} v_1;
layout(location = 0) in float tint_interstage_location0;
layout(location = 1) flat in uint tint_interstage_location1;
layout(location = 2) in float16_t tint_interstage_location2;
layout(location = 3) in f16vec3 tint_interstage_location3;
void tint_store_and_preserve_padding(S value_param) {
  v_1.inner.f = value_param.f;
  v_1.inner.u = value_param.u;
  v_1.inner.v = value_param.v;
  v_1.inner.x = value_param.x;
  v_1.inner.y = value_param.y;
}
void frag_main_inner(S v_2) {
  float f = v_2.f;
  uint u = v_2.u;
  vec4 v = v_2.v;
  float16_t x = v_2.x;
  f16vec3 y = v_2.y;
  tint_store_and_preserve_padding(v_2);
}
void main() {
  frag_main_inner(S(tint_interstage_location0, tint_interstage_location1, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, gl_FragCoord, 0u, 0u, 0u, 0u, tint_interstage_location2, 0u, 0u, 0u, 0u, 0u, 0u, 0u, tint_interstage_location3, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
}
