#version 310 es


struct Input {
  ivec3 position;
  uint tint_pad_0;
};

layout(binding = 0, std430)
buffer input_block_1_ssbo {
  Input inner;
} v;
void main_inner(uvec3 id) {
  ivec3 pos = (v.inner.position - ivec3(0));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_GlobalInvocationID);
}
