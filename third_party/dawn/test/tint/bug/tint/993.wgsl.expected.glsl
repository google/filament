#version 310 es


struct Constants {
  uint zero;
};

struct Result {
  uint value;
};

struct TestData {
  int data[3];
};

layout(binding = 0, std140)
uniform constants_block_1_ubo {
  Constants inner;
} v;
layout(binding = 1, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
layout(binding = 0, std430)
buffer s_block_1_ssbo {
  TestData inner;
} v_2;
int runTest() {
  uint v_3 = min((0u + uint(v.inner.zero)), 2u);
  return atomicOr(v_2.inner.data[v_3], 0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner.value = uint(runTest());
}
