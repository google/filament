#version 310 es

int global = 0;
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
void foo(float x) {
  switch(tint_f32_to_i32(x)) {
    default:
    {
      break;
    }
  }
}
int baz(int x) {
  global = 42;
  return x;
}
void bar(float x) {
  switch(baz(tint_f32_to_i32(x))) {
    default:
    {
      break;
    }
  }
}
void v() {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
