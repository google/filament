#version 310 es


struct UBO {
  ivec4 data[4];
  int dynamic_idx;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
};

struct Result {
  int member_0;
};

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  UBO inner;
} v;
layout(binding = 2, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = min(uint(v.inner.dynamic_idx), 3u);
  v_1.inner.member_0 = v.inner.data[v_2].x;
}
