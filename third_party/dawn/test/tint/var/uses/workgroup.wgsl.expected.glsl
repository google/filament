//
// main1
//
#version 310 es

shared int a;
void uses_a() {
  a = (a + 1);
}
void main1_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    a = 0;
  }
  barrier();
  a = 42;
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main1_inner(gl_LocalInvocationIndex);
}
//
// main2
//
#version 310 es

shared int b;
void uses_b() {
  b = (b * 2);
}
void main2_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    b = 0;
  }
  barrier();
  b = 7;
  uses_b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main2_inner(gl_LocalInvocationIndex);
}
//
// main3
//
#version 310 es

shared int a;
shared int b;
void uses_a() {
  a = (a + 1);
}
void uses_b() {
  b = (b * 2);
}
void uses_a_and_b() {
  b = a;
}
void no_uses() {
}
void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}
void main3_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    a = 0;
    b = 0;
  }
  barrier();
  outer();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main3_inner(gl_LocalInvocationIndex);
}
//
// main4
//
#version 310 es

void no_uses() {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  no_uses();
}
