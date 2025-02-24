#version 310 es
precision highp float;
precision highp int;

in vec4 f_Input;
void g(float a) {
}
void f_inner(vec4 fbf) {
  g(fbf.y);
}
void main() {
  f_inner(f_Input);
}
