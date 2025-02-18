#version 310 es

void original_clusterfuzz_code() {
}
void more_tests_that_would_fail() {
  float a = 1.47112762928009033203f;
  float b = 0.09966865181922912598f;
  float a_1 = 2.5f;
  float b_1 = 2.5f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
