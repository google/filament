#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in vec4 tint_interstage_location0;
in vec4 f_Input;
void g(float a, float b, float c) {
}
void f_inner(vec4 pos, vec4 uv, vec4 fbf) {
  g(pos.x, uv.x, fbf.x);
}
void main() {
  f_inner(gl_FragCoord, tint_interstage_location0, f_Input);
}
