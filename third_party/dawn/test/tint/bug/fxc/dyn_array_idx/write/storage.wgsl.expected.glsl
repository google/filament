#version 310 es


struct UBO {
  int dynamic_idx;
};

struct Result {
  int member_0;
};

struct SSBO {
  int data[4];
};

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  UBO inner;
} v;
layout(binding = 2, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
layout(binding = 1, std430)
buffer ssbo_block_1_ssbo {
  SSBO inner;
} v_2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_3 = min(uint(v.inner.dynamic_idx), 3u);
  v_2.inner.data[v_3] = 1;
  v_1.inner.member_0 = v_2.inner.data[3u];
}
