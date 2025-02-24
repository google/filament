#version 310 es


struct tint_push_constant_struct {
  float user_constant;
  uint tint_first_instance;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
vec4 main_inner(uint b) {
  float v = tint_push_constants.user_constant;
  return vec4((v + float(b)));
}
void main() {
  uint v_1 = uint(gl_InstanceID);
  vec4 v_2 = main_inner((v_1 + tint_push_constants.tint_first_instance));
  gl_Position = vec4(v_2.x, -(v_2.y), ((2.0f * v_2.z) - v_2.w), v_2.w);
  gl_PointSize = 1.0f;
}
