#version 310 es

layout(binding = 0, std430)
buffer rarr_block_1_ssbo {
  float inner[];
} v;
void vector() {
  int idx = 3;
  int x = ivec2(1, 2)[min(uint(idx), 1u)];
}
void matrix() {
  int idx = 4;
  vec2 x = mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f))[min(uint(idx), 1u)];
}
void fixed_size_array() {
  int arr[2] = int[2](1, 2);
  int idx = 3;
  int x = arr[min(uint(idx), 1u)];
}
void runtime_size_array() {
  int idx = -1;
  uint v_1 = (uint(v.inner.length()) - 1u);
  uint v_2 = min(uint(idx), v_1);
  float x = v.inner[v_2];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vector();
  matrix();
  fixed_size_array();
  runtime_size_array();
}
