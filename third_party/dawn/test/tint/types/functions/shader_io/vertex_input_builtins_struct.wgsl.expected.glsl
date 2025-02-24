#version 310 es


struct tint_push_constant_struct {
  uint tint_first_instance;
};

struct VertexInputs {
  uint vertex_index;
  uint instance_index;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
vec4 main_inner(VertexInputs inputs) {
  uint foo = (inputs.vertex_index + inputs.instance_index);
  return vec4(0.0f);
}
void main() {
  uint v = uint(gl_VertexID);
  uint v_1 = uint(gl_InstanceID);
  vec4 v_2 = main_inner(VertexInputs(v, (v_1 + tint_push_constants.tint_first_instance)));
  gl_Position = vec4(v_2.x, -(v_2.y), ((2.0f * v_2.z) - v_2.w), v_2.w);
  gl_PointSize = 1.0f;
}
