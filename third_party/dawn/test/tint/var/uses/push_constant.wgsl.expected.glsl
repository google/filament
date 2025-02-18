//
// main1
//
#version 310 es


struct tint_push_constant_struct {
  int inner;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
void uses_a() {
  int foo = tint_push_constants.inner;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_a();
}
//
// main2
//
#version 310 es


struct tint_push_constant_struct {
  int inner;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
void uses_a() {
  int foo = tint_push_constants.inner;
}
void uses_uses_a() {
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_uses_a();
}
//
// main3
//
#version 310 es


struct tint_push_constant_struct {
  int inner;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
void uses_b() {
  int foo = tint_push_constants.inner;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_b();
}
//
// main4
//
#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
