#version 310 es


struct Out {
  vec4 pos;
};

Out main_inner() {
  return Out(vec4(0.0f));
}
void main() {
  vec4 v = main_inner().pos;
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
