#version 310 es

vec4 v(uint v_1) {
  return vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
void main() {
  vec4 v_2 = v(uint(gl_VertexID));
  gl_Position = vec4(v_2.x, -(v_2.y), ((2.0f * v_2.z) - v_2.w), v_2.w);
  gl_PointSize = 1.0f;
}
