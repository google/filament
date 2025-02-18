#version 310 es


struct main_out {
  vec4 member_0;
};

vec4 v = vec4(0.0f);
void main_1() {
  v = vec4(0.0f);
}
main_out main_inner() {
  main_1();
  return main_out(v);
}
void main() {
  vec4 v_1 = main_inner().member_0;
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
