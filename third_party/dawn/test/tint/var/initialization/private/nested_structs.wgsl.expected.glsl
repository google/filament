#version 310 es


struct S1 {
  int i;
};

struct S2 {
  S1 s1;
};

struct S3 {
  S2 s2;
};

S3 P = S3(S2(S1(42)));
layout(binding = 0, std430)
buffer out_block_1_ssbo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = P.s2.s1.i;
}
