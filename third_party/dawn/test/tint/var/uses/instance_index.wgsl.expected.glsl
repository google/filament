#version 310 es


struct tint_push_constant_struct {
  uint tint_first_instance;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
vec4 main_inner(uint b) {
  return vec4(float(b));
}
void main() {
  uint v = uint(gl_InstanceID);
  vec4 v_1 = main_inner((v + tint_push_constants.tint_first_instance));
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
