#version 310 es


struct UniformBuffer {
  ivec3 d;
  uint tint_pad_0;
};

layout(binding = 0, std140)
uniform u_input_block_1_ubo {
  UniformBuffer inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec3 temp = (v.inner.d << (uvec3(0u) & uvec3(31u)));
}
