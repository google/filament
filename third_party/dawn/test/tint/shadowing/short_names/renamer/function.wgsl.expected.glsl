#version 310 es

int v() {
  return 0;
}
float v_1(int v_2) {
  return float(v_2);
}
bool v_3(float v_4) {
  return bool(v_4);
}
vec4 v_5(uint v_6) {
  return mix(vec4(0.0f), vec4(1.0f), bvec4(v_3(v_1(v()))));
}
void main() {
  vec4 v_7 = v_5(uint(gl_VertexID));
  gl_Position = vec4(v_7.x, -(v_7.y), ((2.0f * v_7.z) - v_7.w), v_7.w);
  gl_PointSize = 1.0f;
}
