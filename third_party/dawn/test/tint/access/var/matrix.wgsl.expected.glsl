#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3 m = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 v = m[1u];
  float f = v.y;
  v_1.inner = f;
}
