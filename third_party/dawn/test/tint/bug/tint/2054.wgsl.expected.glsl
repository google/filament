#version 310 es

layout(binding = 0, std430)
buffer out_block_1_ssbo {
  float inner;
} v;
void bar(inout float p) {
  float a = 1.0f;
  float b = 2.0f;
  bool v_1 = false;
  if ((a >= 0.0f)) {
    v_1 = (b >= 0.0f);
  } else {
    v_1 = false;
  }
  bool cond = v_1;
  p = mix(a, b, cond);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float param = 0.0f;
  bar(param);
  v.inner = param;
}
