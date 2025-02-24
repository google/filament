#version 310 es
precision highp float;
precision highp int;

in vec4 f_Input;
in vec4 f_Input_1;
void g(float a, float b) {
}
void f_inner(vec4 fbf_1, vec4 fbf_3) {
  g(fbf_1.x, fbf_3.y);
}
void main() {
  f_inner(f_Input, f_Input_1);
}
