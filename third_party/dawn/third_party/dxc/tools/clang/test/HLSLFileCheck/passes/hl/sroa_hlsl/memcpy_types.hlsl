// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Regression test a bug where passing array struct element to function causes
// a crash due to memcpy src and dest type mismatch.

// CHECK: @main
 
struct my_struct
{
  float x : POSITION0;
  float y : TEXCOORD0;
};
 
uint foo(my_struct s)
{
  return 0;
}
 
void main(my_struct a[1])
{
  uint r = foo(a[0]);
}
