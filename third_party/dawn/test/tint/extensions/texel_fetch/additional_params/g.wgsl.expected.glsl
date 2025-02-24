#version 310 es
precision highp float;
precision highp int;

in vec4 f_Input;
void g(float a, float b) {
}
void f_inner(vec4 fbf, vec4 pos) {
  g(fbf.w, pos.x);
}
void main() {
  f_inner(f_Input, gl_FragCoord);
}
