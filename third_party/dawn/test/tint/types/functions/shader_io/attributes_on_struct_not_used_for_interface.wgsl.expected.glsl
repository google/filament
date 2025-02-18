#version 310 es
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
  uint tint_pad_34;
  uint tint_pad_35;
  uint tint_pad_36;
  uint tint_pad_37;
  uint tint_pad_38;
  uint tint_pad_39;
  uint tint_pad_40;
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
  uint tint_pad_55;
  uint tint_pad_56;
  uint tint_pad_57;
};

layout(binding = 0, std430)
buffer f_output_block_ssbo {
  S inner;
} v_1;
void tint_store_and_preserve_padding(S value_param) {
  v_1.inner.f = value_param.f;
  v_1.inner.u = value_param.u;
  v_1.inner.v = value_param.v;
}
void main() {
  tint_store_and_preserve_padding(S(1.0f, 2u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, vec4(3.0f), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
}
