#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in vec4 tint_interstage_location0;
layout(location = 1) flat in vec4 tint_interstage_location1;
in vec4 f_Input;
void g(float a, float b, float c) {
}
void f_inner(vec4 a, vec4 b, vec4 fbf) {
  g(a.x, b.y, fbf.x);
}
void main() {
  f_inner(tint_interstage_location0, tint_interstage_location1, f_Input);
}
