#version 310 es
precision highp float;
precision highp int;


struct frexp_result_f32 {
  float f;
};

struct frexp_result_f32_1 {
  float member_0;
  int member_1;
};

frexp_result_f32 a = frexp_result_f32(0.0f);
frexp_result_f32_1 b = frexp_result_f32_1(0.5f, 1);
layout(location = 0) out vec4 main_loc0_Output;
vec4 main_inner() {
  return vec4(a.f, b.member_0, 0.0f, 0.0f);
}
void main() {
  main_loc0_Output = main_inner();
}
