#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 pos;
};

in vec4 f_Input;
void g(float a, float b) {
}
void f_inner(In v, vec4 fbf) {
  g(v.pos.x, fbf.y);
}
void main() {
  In v_1 = In(gl_FragCoord);
  f_inner(v_1, f_Input);
}
