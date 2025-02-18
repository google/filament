#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 uv;
};

in vec4 f_Input;
layout(location = 0) in vec4 tint_interstage_location0;
void g(float a, float b, float c) {
}
void f_inner(vec4 pos, vec4 fbf, In v) {
  g(pos.x, fbf.x, v.uv.x);
}
void main() {
  vec4 v_1 = gl_FragCoord;
  vec4 v_2 = f_Input;
  f_inner(v_1, v_2, In(tint_interstage_location0));
}
