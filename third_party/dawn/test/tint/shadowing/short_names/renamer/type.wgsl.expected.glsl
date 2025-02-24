#version 310 es


struct tint_struct {
  int member_0;
};

vec4 v(uint v_1) {
  tint_struct v_2 = tint_struct(1);
  float v_3 = float(v_2.member_0);
  bool v_4 = bool(v_3);
  return mix(vec4(0.0f), vec4(1.0f), bvec4(v_4));
}
void main() {
  vec4 v_5 = v(uint(gl_VertexID));
  gl_Position = vec4(v_5.x, -(v_5.y), ((2.0f * v_5.z) - v_5.w), v_5.w);
  gl_PointSize = 1.0f;
}
