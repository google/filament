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

layout(binding = 0, std430)
buffer out_block_1_ssbo {
  int inner;
} v;
int f(S3 s3) {
  return s3.s2.s1.i;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = f(S3(S2(S1(42))));
}
